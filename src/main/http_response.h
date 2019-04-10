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

#ifndef __HTTP_RESPONSE_H__
#define __HTTP_RESPONSE_H__

#include <string>
#include <vector>
#include <memory>

#include "misc.h"
#include "logger.h"
#include "stream.h"
#include "stream_socket.h"
#include "http_status.h"
#include "http_header.h"
#include "http_request.h"
#include "compression.h"

//--------------------------------------------------------------
// HTTP response.
//--------------------------------------------------------------

class HttpResponse : public OutputStream {
public:
    HttpResponse(HttpRequest const & request, StreamSocket & socket, bool keepalive);
    ~HttpResponse();

    void    write(void const * data, size_t length) override;
    void    flush() override;

    date    getResponseDate();
    void    setHttpStatus(HttpStatus status)            { httpStatus_ = status; }

private:
    HttpRequest const &                         request_;               // info about the request
    StreamSocket &                              socket_;                // client socket 
    std::vector<std::unique_ptr<OutputStream>>  transformers_;          // list of transformers applied to the response (compression, chunk, etc.)
    HttpStatus                                  httpStatus_;            // HTTP status of the response
    int                                         headerState_;           // (temporary variable to parse the headers) machine state
    std::string                                 headerKey_;             // (temporary variable to parse the headers) key being parsed
    std::string                                 headerValue_;           // (temporary variable to parse the headers) value being parsed
    HttpHeaderMap                               headers_;               // response headers
    bool                                        keepAlive_;             // send a "connection: close/keepalive"
    compression::mode                           encoding_;              // actual encoding
    date                                        responseDate_;          // date of the response
    logger::dump                                dump_;                  // helper object to dump response body

    void    prepareForBody();
    void    emitHeaders(long length);
};

//--------------------------------------------------------------

#endif

//========================================================================
