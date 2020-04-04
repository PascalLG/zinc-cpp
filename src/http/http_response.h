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

#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string>
#include <vector>
#include <memory>

#include "../misc/date.h"
#include "../misc/logger.h"
#include "ihttpconfig.h"
#include "compression.h"
#include "stream_socket.h"
#include "http_status.h"
#include "http_verb.h"
#include "http_header.h"
#include "http_request.h"

//--------------------------------------------------------------
// HTTP response.
//--------------------------------------------------------------

class HttpResponse : public OutputStream {
public:
    enum class Connection {
        Close,
        KeepAlive,
        Upgrade,
    };

    HttpResponse(IHttpConfig & config, HttpRequest const & request, StreamSocket & socket, Connection connection);
    ~HttpResponse() override;

    bool    write(void const * data, size_t length) override;
    bool    flush() override;

    date    getResponseDate();
    void    setHttpStatus(HttpStatus status)            { httpStatus_ = status; }

private:
    IHttpConfig &                               config_;                // server configuration
    HttpRequest const &                         request_;               // info about the request
    StreamSocket &                              socket_;                // client socket 
    std::vector<std::unique_ptr<OutputStream>>  transformers_;          // list of transformers applied to the response (compression, chunk, etc.)
    HttpStatus                                  httpStatus_;            // HTTP status of the response
    int                                         headerState_;           // (temporary variable to parse the headers) machine state
    std::string                                 headerKey_;             // (temporary variable to parse the headers) key being parsed
    std::string                                 headerValue_;           // (temporary variable to parse the headers) value being parsed
    HttpHeaderMap                               headers_;               // response headers
    Connection                                  connection_;            // send a "connection: close/keepalive/upgrade"
    compression::mode                           encoding_;              // actual encoding
    date                                        responseDate_;          // date of the response
    logger::dump                                dump_;                  // helper object to dump response body

    void    prepareForBody();
    void    emitHeaders(long length);
};

//--------------------------------------------------------------

#endif

//========================================================================
