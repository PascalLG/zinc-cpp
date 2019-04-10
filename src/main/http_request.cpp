//========================================================================
// Zinc - Web Server
// Copyright (c) 2019, Pascal Levy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//========================================================================

#include <cctype>
#include <iostream>
#include <algorithm>

#include "logger.h"
#include "config.h"
#include "misc.h"
#include "http_request.h"

//========================================================================
// HttpRequest
//
// Represent an HTTP request. Parse the data the client sends and store
// the protocol version, the verb, the URI, the headers and the body.
// Provide methods to retrieve infos about the request.
//========================================================================

int const   HTTP_VERSION_0_9    = 0x0009;
int const   HTTP_VERSION_1_0    = 0x0100;
int const   HTTP_VERSION_1_1    = 0x0101;

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

HttpRequest::HttpRequest(AddrIn const & local, AddrIn const & remote, bool secure)
  : localAddress_(local),
    remoteAddress_(remote),
    secure_(secure),
    verb_(),
    headers_(),
    httpVersion_(HTTP_VERSION_0_9),
    body_() {

    LOG_TRACE("Init HttpRequest");
}

//--------------------------------------------------------------
// Destructor.
//--------------------------------------------------------------

HttpRequest::~HttpRequest() {
    LOG_TRACE("Destroy HttpRequest");
}

//--------------------------------------------------------------
// Parse a request.
//--------------------------------------------------------------

HttpRequest::Result HttpRequest::parse(InputStream & s) {

    // Parse the request line and headers. Abort as soon
    // as an error occurs, don't try to recover: we will
    // simply force a connection close.

    Configuration const & configuration = Configuration::getInstance();
    int timeout = configuration.getTimeout() * 1000;

    Result r1 = parseRequestLine(s, timeout, configuration.getLimitRequestLine());
    if (!r1.isOK()) {
        return r1;
    }
    LOG_INFO_RECV("Requesting: " << verb_ << " " << uri_.getPath() << " HTTP/" << (httpVersion_ >> 8) << "." << (httpVersion_ & 0xFF));

    Result r2 = parseHeaders(s, timeout, configuration.getLimitRequestHeaders());
    if (!r2.isOK()) {
        return r2;
    }

    // Determine if a body is present, and how it is transfered
    // (i.e. chunked or not)

    std::function<bool(OutputStream &)> reader;
    HttpHeaderMap::const_iterator got;

    if ((got = headers_.find(HttpHeader::TransferEncoding)) != headers_.end()) {
        if (got->second != "identity") {
            return Result::Error(501);  // transfer encodings other than identity are not supported yet
        }
    }

    if ((got = headers_.find(HttpHeader::ContentLength)) != headers_.end()) {
        long length = string::to_long(got->second, 10);
        if (length < 0) {
            return Result::Error(400);
        }
        if (length > static_cast<long>(configuration.getLimitRequestBody())) {
            return Result::Error(413);
        }

        reader = [length, timeout, &s] (OutputStream & body) {
            logger::dump dump(ansi::cyan, "<=");
            size_t rem = length;
            while (rem) {
                char buffer[1024];
                size_t r = s.read(buffer, std::min(rem, sizeof(buffer)), timeout, false);
                if (!r) {
                    return false;
                }
                body.write(buffer, r);
                dump.write(buffer, r);
                rem -= r;
            }
            body.flush();
            return true;
        };
    }

    // A body is present: read it.

    if (reader) {
        Body wrapper(body_);

        // (later: insert gzip/inflate/brotli decompression here)

        if (!reader(wrapper)) {
            return Result::Error(400);
        }
        LOG_DEBUG_RECV("<= request body (" << body_.getSize() << " bytes)");
    }

    return Result::OK();
}

//--------------------------------------------------------------
// Indicates if we should try to keep the connection alive, 
// depending on the header fields and protocol version.
//--------------------------------------------------------------

bool HttpRequest::shouldKeepAlive() const {
    std::string connection;
    auto got = headers_.find(HttpHeader::Connection);
    if (got != headers_.end()) {
        connection = got->second;
        string::lowercase(connection);
    }
    return connection != "close" && (httpVersion_ >= HTTP_VERSION_1_1 || connection == "keep-alive");
}

//--------------------------------------------------------------
// Return the list of accepted encodings.
//--------------------------------------------------------------

compression::set HttpRequest::getAcceptedEncodings() const {
    return parseAcceptedEncodings(getHeaderValue(HttpHeader::AcceptEncoding));
}

//--------------------------------------------------------------
// Return the value for a given header, or an empty string if
// this header is missing.
//--------------------------------------------------------------

std::string const & HttpRequest::getHeaderValue(HttpHeader const & hdr) const {
    auto got = headers_.find(hdr);
    return got != headers_.end() ? got->second : string::empty;
}

//--------------------------------------------------------------
// Parse the request line. A finite state machine is used to parse
// the whole line in one pass, extracting the verb, the uri,
// the HTTP version on the fly.
//
// The parser tolerates some deviations from the standard:
// - it accepts LF instead of CRLF as line endings
// - it treats TAB as SP
// - it accepts consecutive blanks where only one is expected
// - blanks at the end of a line are silently ignored
//--------------------------------------------------------------

HttpRequest::Result HttpRequest::parseRequestLine(InputStream & s, int timeout, size_t maxsize) {
    std::string buffer;
    int state = 0, ch = 0, val1 = 0, val2 = 0;
    bool skip = false;

    for (size_t count = 0; count < maxsize; ) {

        // Read the next character, if necessary.

        if (!skip) {
            ch = s.readByte(timeout);
            if (ch < 0) {
                return Result::Abort();             // timeout or socket closed
            }
            count++;
        } else {
            skip = false;
        }

        // Determine what to do, depending on the current
        // state and character.

        switch (state) {
        case 0:                                     // read the first character of the verb
            if (isalpha(ch)) {
                buffer.push_back(ch);
                state = 1;
            } else {
                state = 13;
            }
            break;
        case 1:                                     // read the verb
            if (isalpha(ch)) {
                buffer.push_back(ch);
            } else if (isblank(ch)) {
                LOG_TRACE("Parsed verb: " << buffer);
                verb_ = HttpVerb(buffer);
                buffer.clear();
                state = 2;
            } else {
                state = 13;
            }
            break;
        case 2:                                     // skip blanks
            if (!isblank(ch)) {
                state = 3;
                skip = true;
            }
            break;
        case 3:                                     // read the URI
            if (isgraph(ch)) {
                buffer.push_back(ch);
            } else {
                LOG_TRACE("Parsed URI: " << buffer);
                if (uri_.parse(buffer)) {
                    if (isblank(ch)) {
                        state = 4;
                    } else if (ch == '\r') {
                        state = 12;
                    } else if (ch == '\n') {
                        return Result::OK();
                    } else {
                        state = 13;
                    }
                } else {
                    state = 13;
                }
            }
            break;
        case 4:                                     // skip blanks
            if (!isblank(ch)) {
                state = 5;
                skip = true;
            }
            break;
        case 5:
            if (ch == 'H' || ch == 'h') {           // read a 'H' or a CRLF
                state = 6;   
            } else if (ch == '\r') {
                state = 12;
            } else if (ch == '\n') {
                return Result::OK();
            } else {
                state = 13;
            }
            break;
        case 6:                                     // read a 'T'
            state = ch == 'T' || ch == 't' ? 7 : 13;
            break;
        case 7:                                     // read a 'T'
            state = ch == 'T' || ch == 't' ? 8 : 13;
            break;
        case 8:                                     // read a 'P'
            state = ch == 'P' || ch == 'p' ? 9 : 13;
            break;
        case 9:                                     // read a '/'
            state = ch == '/' ? 10 : 13;
            break;
        case 10:                                    // read major version number
            if (ch >= '0' && ch <= '9') {
                val1 *= 10;
                val1 += ch - '0';
            } else if (ch == '.') {
                state = 11;
            } else {
                state = 13;
            }
            break;
        case 11:                                    // read minor version number
            if (ch >= '0' && ch <= '9') {
                val2 *= 10;
                val2 += ch - '0';
                httpVersion_ = (val1 << 8) | val2;
            } else if (ch == '\r' || isblank(ch)) {
                state = 12;
            } else if (ch == '\n') {
                return Result::OK();
            } else {
                state = 13;
            }
            break;
        case 12:                                    // skip blanks and process CRLF
            if (ch == '\n') {
                return Result::OK();
            } else if (!isspace(ch)) {
                state = 13;
            }
            break;
        case 13:                                    // error recovery
            if (ch == '\n') {
                return Result::Error(400);
            }
            break;
        }
    }
    return Result::Error(414);                      // URI too long
}

//--------------------------------------------------------------
// Parse headers. A finite state machine is used to parse the
// whole header section in one pass, populating a dictionary
// of key/value pairs on the fly.
//
// The parser tolerates some deviations from the standard:
// - it accepts LF instead of CRLF as line endings
// - it treats TAB as SP
// - it accepts consecutive blanks where only one is expected
// - blanks at the end of a line are silently ignored
//--------------------------------------------------------------

HttpRequest::Result HttpRequest::parseHeaders(InputStream & s, int timeout, size_t maxsize) {
    std::string key, value;
    int state = 0, ch = 0;
    bool skip = false;
    Result result = Result::OK();

    for (size_t count = 0; count < maxsize; ) {

        // Read the next character, if necessary.

        if (!skip) {
            ch = s.readByte(timeout);
            if (ch < 0) {
                return Result::Abort();             // timeout or socket closed
            }
            count++;
        } else {
            skip = false;
        }

        // Determine what to do, depending on the current
        // state and character.

        switch (state) {
        case 0:                                     // read either a CRLF or a key/value pair
            if (ch == '\r' || isblank(ch)) {
                state = 1;
            } else if (ch == '\n') {
                return result;
            } else if (isalnum(ch)) {
                key.clear();
                key.push_back(ch);
                state = 2;
            } else {
                state = 6;
            }
            break;
        case 1:                                     // skip blanks and process CRLF
            if (ch == '\n') {
                return result;
            } else if (!isspace(ch)) {
                state = 6;
            }
            break;
        case 2:                                     // read a key up to the next ':'
            if (ch == ':') {
                state = 3;
            } else if (isgraph(ch)) {
                key.push_back(ch);
            } else {
                state = 6;
            }
            break;
        case 3:                                     // skip blanks
            if (!isblank(ch)) {
                value.clear();
                state = 4;
                skip = true;
            }
            break;
        case 4:                                     // read value up to the end of line.
            if (isprint(ch)) {
                value.push_back(ch);
            } else if (ch == '\r') {
                state = 5;
            } else if (ch == '\n') {
                state = 5;
                skip = true;
            } else {
                state = 6;
            }
            break;
        case 5:                                     // process CRLF
            if (ch == '\n') {
                string::trim(value, string::trim_right);
                LOG_DEBUG_RECV("<= " << key << ": " << value);
                headers_.emplace(key, value);
                state = 0;
            } else {
                value.push_back('\r');
                state = 4;
                skip = true;
            }
            break;
        case 6:                                     // error recovery
            if (ch == '\n') {
                result = Result::Error(400);        // Bad request
                state = 0;
            }
            break;
        }
    }
    return Result::Error(431);                      // Request header fields too large
}

//========================================================================
