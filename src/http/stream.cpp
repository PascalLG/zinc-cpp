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

#include <cstring>

#include "../misc/logger.h"
#include "stream.h"

//========================================================================
// InputStream
//
// Interface for a class that implements an input stream, i.e. object we
// can read from. Derived classes must implement the read() method. As a
// convenience, an implementation of a readByte method is provided.
//========================================================================

//--------------------------------------------------------------
// Helper function to read one byte. Return a negative value in
// case of failure.
//--------------------------------------------------------------

int InputStream::readByte(std::chrono::milliseconds timeout) {
    unsigned char ch;
    return read(&ch, 1, timeout, true) == 1 ? ch : -1;
}

//========================================================================
// OutputStream
//
// Interface for a class that implements an output stream, i.e. object we
// can write to. Derived classes must implement the write() method. Several
// convenience methods are provided to write CRLF, strings, HTTP headers,
// and so on.
//
// Some streams are actually filters, for example to compress or change
// the encoding of the data stream. Such filters can be chained via the
// Destination property.
//========================================================================

//--------------------------------------------------------------
// Flush the stream. The default implementation does nothing.
//--------------------------------------------------------------

bool OutputStream::flush() {
    return true;
}

//--------------------------------------------------------------
// Emit a CRLF. (Note: the end-of-line marker in HTTP messages
// is always CRLF, even on UNIX systems.)
//--------------------------------------------------------------

void OutputStream::emitEol() {
    write("\r\n", 2);
}

//--------------------------------------------------------------
// Emit a HTTP header, i.e. a line of the form: "key: value\r\n"
//--------------------------------------------------------------

void OutputStream::emitHeader(HttpHeader const & header, std::string const & value) {
    std::string const & key = header.getFieldName();
    write(key.data(), key.length());
    write(": ", 2);
    write(value.data(), value.length());
    emitEol();
}

//--------------------------------------------------------------
// Emit a null-terminated string.
//--------------------------------------------------------------

void OutputStream::emitPage(char const * text) {
    write(text, strlen(text));
}

//--------------------------------------------------------------
// Process a page template and emit the resulting page. A template
// is simply text with tags of the form: {{name}}. Each time
// we encounter a tag, the callback provided by the caller is
// invoked with the name of that tag, and the string it returns
// is emitted.
//--------------------------------------------------------------

void OutputStream::emitPage(char const * text, std::function<std::string(std::string const &)> fields) {
    std::unordered_map<std::string, std::string> cache;
    for (char const * p = text; ; ) {
        char const * q1 = strstr(p, "{{");
        if (q1) {
            write(p, static_cast<size_t>(q1 - p));
            char const * q2 = strstr(q1 + 2, "}}");
            if (q2) {
                std::string fld(q1 + 2, static_cast<size_t>(q2 - q1 - 2));
                std::string value;
                auto got = cache.find(fld);
                if (got == cache.end()) {
                    value = fields(fld);
                    cache.emplace(fld, value);
                } else {
                    value = got->second;
                }
                write(value.data(), value.length());
                p = q2 + 2;
            } else {
                LOG_ERROR("Internal error: unclosed tag in template");
                break;
            }
        } else {
            write(p, strlen(p));
            break;
        }
    }
}

//========================================================================
