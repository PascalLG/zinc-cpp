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

#include "../misc/string.h"
#include "mimetype.h"
#include "stream_chunked.h"
#include "stream_null.h"
#include "http_response.h"

//========================================================================
// HttpResponse
//
// This class is responsible for building, encoding and transmitting
// the response to the client. It behaves as a filter: the local resource
// (which can be a static file, the result of a CGI script, a built-in
// page, etc.) writes to this object, which in turn on the fly adds, alters
// or removes header fields, encodes the data, and sends the response to
// the client.
//
// First the resource data is directed to a state machine that parses
// the header fields. When the header is done, this object determines
// how to encode/transmit the response body (see prepareForBody). When
// the machinery is ready, headers are transmitted (see emitHeaders)
// then the resource data are processed (see the last lines of code in
// write).
//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

HttpResponse::HttpResponse(IHttpConfig & config, HttpRequest const & request, StreamSocket & socket, Connection connection)
  : config_(config),
    request_(request),
    socket_(socket),
    httpStatus_(200), 
    headerState_(0),
    connection_(connection),
    encoding_(compression::none),
    dump_(ansi::magenta, "=>") {
    LOG_TRACE("Init HttpResponse");
}

//--------------------------------------------------------------
// Destructor.
//--------------------------------------------------------------

HttpResponse::~HttpResponse() {
    LOG_TRACE("Destroy HttpResponse");
}

//--------------------------------------------------------------
// The resource being transmitted "pushes" its data by calling
// this method repeatedly. This method is actually a state
// machine: depending on the state, the data are either parsed
// as a list of header fields, or either transmitted to the
// client.
//--------------------------------------------------------------

bool HttpResponse::write(void const * data, size_t length) {
    char const * text = static_cast<char const *>(data);
    size_t ndx = 0;

    // Parse the header. We try to recognize a list of
    // CRLF separated "key: value" pairs, terminated
    // by an empty line.

    while (headerState_ < 10 && ndx < length) {
        int ch = text[ndx++];
        switch (headerState_) {
        case 0:
            if (isalnum(ch) || ch == '-') {
                headerKey_.push_back(static_cast<char>(ch));
            } else if (ch == ':') {
                headerState_ = 2;
            } else if (ch == '\r') {
                headerState_ = 1;
            } else if (ch == '\n') {
                prepareForBody();       // empty line recognized
                headerState_ = 10;
            } else {
                headerState_ = 999;
            }
            break;
        case 1:
            if (ch == '\n') {
                headerState_ = 0;
                ndx--;
            } else {
                headerState_ = 999;
            }
            break;
        case 2:
            if (!isblank(ch)) {
                headerState_ = 3;
                ndx--;
            }
            break;
        case 3:
            if (ch == '\r') {
                headerState_ = 4;
            } else if (ch == '\n') {
                string::trim(headerValue_, string::trim_right);
                headers_[headerKey_] = headerValue_;
                headerKey_.clear();
                headerValue_.clear();
                headerState_ = 0;
            } else {
                headerValue_.push_back(static_cast<char>(ch));
            }
            break;
        case 4:
            if (ch == '\n') {
                headerState_ = 3;
                ndx--;
            } else {
                headerState_ = 999;
            }
            break;
        }
    }

    // Header parsing is done. Just transmit the data
    // "as-is" to the destination.

    bool ok = true;
    if (headerState_ == 10 && ndx < length) {
        ok = getDestination()->write(text + ndx, length - ndx);
        dump_.write(text + ndx, length - ndx);
    }

    return ok;
}

//--------------------------------------------------------------
// The resource being transmitted calls this method when it is
// done transmitting its content.
//--------------------------------------------------------------

bool HttpResponse::flush() {
    if (headerState_ == 10) {
        getDestination()->flush();  // Optimal case: flush the destination stream.
    } else {
        headers_.clear();           // Error case: the resource failed to send valid headers/content.
        emitHeaders(0);             // We reply an empty page.
    }
    return true;
}

//--------------------------------------------------------------
// Return the response date. This date corresponds to the moment
// this function is called for the first time, which depending
// on the actual resource being transmitted is approximately
// the moment the headers are starting to be sent.
//--------------------------------------------------------------

date HttpResponse::getResponseDate() {
    if (!responseDate_.valid()) {
        responseDate_ = date::now();
    }
    return responseDate_;
}

//--------------------------------------------------------------
// Prepare the object for transmitting the response body, after
// parsing the headers is done.
//--------------------------------------------------------------

void HttpResponse::prepareForBody() {

    // The destination device is the client socket for regular
    // requests and null device for HEAD requests.

    if (request_.getVerb().isOneOf(HttpVerb::Head)) {
        std::unique_ptr<OutputStream> null = std::make_unique<StreamNull>();
        setDestination(null.get());
        transformers_.push_back(std::move(null));
    } else {
        setDestination(&socket_);
    }

    // Determine if the resource transmitted a fixed length.

    auto got = headers_.find(HttpHeader::ContentLength);
    long length = got != headers_.end() ? string::to_long(got->second, 10) : -1;

    // If HTTP compression is enabled and the client accepts compression and
    // the resource is either bigger than 16 bytes or either of unknown size,
    // apply compression.

    compression::set accepted = request_.getAcceptedEncodings();
    if (config_.isCompressionEnabled() && !accepted.empty() && (length < 0 || length >= 16)) {
        auto got2 = headers_.find(HttpHeader::ContentType);
        if (got2 != headers_.end()) {
            encoding_ = selectCompressionMode(accepted, Mime(got2->second));
        }
    }

    // Build the chain of stream transformers that will encode
    // the response body. If compression is enabled or if size
    // is unknown, we choose chunked encoding and headers are
    // delayed. Otherwise, we send headers now and there
    // is no transformers.

    if (encoding_ != compression::none || length < 0) {
        std::unique_ptr<OutputStream> transformer1 = std::make_unique<StreamChunked>([this](long length) { this->emitHeaders(length); });
        transformer1->setDestination(getDestination());
        setDestination(transformer1.get());
        transformers_.push_back(std::move(transformer1));

        if (encoding_ != compression::none) {
            std::unique_ptr<OutputStream> transformer2 = makeStreamTransformer(encoding_, length);
            transformer2->setDestination(getDestination());
            setDestination(transformer2.get());
            transformers_.push_back(std::move(transformer2));
        }
    } else {
        emitHeaders(length);
    }
}

//--------------------------------------------------------------
// Emit the headers.
//--------------------------------------------------------------

void HttpResponse::emitHeaders(long length) {

    // If the resource transmitted a status, set the HTTP
    // status accordingly and remove the corresponding
    // header (it must not be transmitted to the client).

    auto got = headers_.find(HttpHeader::Status);
    if (got != headers_.end()) {
        std::string const & st = got->second;
        size_t len = st.size();
        if (len == 3 || (len > 3 && isspace(st[3]))) {
            long status = string::to_long(st.substr(0, 3), 10);
            if (status >= 100 && status <= 999) {
                setHttpStatus(status);
            }
        }
        headers_.erase(HttpHeader::Status);
    }

    // Build and transmit the response line.

    std::string resp = "HTTP/1.1 " + std::to_string(httpStatus_.getStatusCode()) + " " + httpStatus_.getStatusString();
    socket_.write(resp.data(), resp.length());
    socket_.emitEol();
    LOG_DEBUG_SEND("=> " << resp);

    // If the length is known, insert a Content-Length field
    // and remove any Transfer-Encoding indication. Otherwise,
    // indicate a chunked encoding.

    if (length >= 0) {
        headers_[HttpHeader::ContentLength] = std::to_string(length);
        headers_.erase(HttpHeader::TransferEncoding);
    } else {
        headers_.erase(HttpHeader::ContentLength);
        headers_[HttpHeader::TransferEncoding] = "chunked";
    }

    // Set the field indicating the compression mode.

    if (encoding_ == compression::none) {
        headers_.erase(HttpHeader::ContentEncoding);
    } else {
        headers_[HttpHeader::ContentEncoding] = getCompressionName(encoding_);
    }

    // Set the field indicating the connection status.

    switch (connection_) {
    case Connection::Close:     headers_[HttpHeader::Connection] = "close";     break;
    case Connection::Upgrade:   headers_[HttpHeader::Connection] = "upgrade";   break;
    default:                    headers_.erase(HttpHeader::Connection);         break;  // in HTTP 1.1, no connection indication = keep alive
    }

    // Add miscellaneous headers.

    headers_[HttpHeader::Server] = config_.getVersionString();
    headers_[HttpHeader::Date] = getResponseDate().to_http();

    // Send all the headers to the client.

    for (auto & p: headers_) {
        socket_.emitHeader(p.first, p.second);
        LOG_DEBUG_SEND("=> " << p.first.getFieldName() << ": " << p.second);
    }
    socket_.emitEol();
}

//========================================================================
