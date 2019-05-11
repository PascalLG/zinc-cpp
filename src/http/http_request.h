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

#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#include <string>

#include "../misc/filesys.h"
#include "uri.h"
#include "http_header.h"
#include "http_status.h"
#include "http_verb.h"
#include "compression.h"
#include "stream.h"
#include "stream_socket.h"

class IConfig;

//--------------------------------------------------------------
// HTTP request.
//--------------------------------------------------------------

class HttpRequest {
public:
    HttpRequest(AddrIn const & local, AddrIn const & remote, bool secure);
    ~HttpRequest();

    class Result {
    public:
        static Result       OK()                            { return Result(0, 0);      }
        static Result       Abort()                         { return Result(1, 0);      }
        static Result       Error(int status)               { return Result(2, status); }

        bool                isOK() const                    { return code_ == 0;        }
        bool                isAborted() const               { return code_ == 1;        }
        bool                isError() const                 { return code_ == 2;        }
        HttpStatus const  & getHttpStatus() const           { return status_;           }

    private:
        Result(int code, int status) : code_(code), status_(status)   { }

        int         code_;
        HttpStatus  status_;
    };

    Result                  parse(IConfig & config, InputStream & s);
    bool	                shouldKeepAlive() const;
    compression::set        getAcceptedEncodings() const;
    std::string const &     getHeaderValue(HttpHeader const & hdr) const;

    AddrIn const &          getLocalAddress() const         { return localAddress_;     }
    AddrIn const &          getRemoteAddress() const        { return remoteAddress_;    }
    HttpVerb const &        getVerb() const                 { return verb_;             }
    URI const &             getURI() const                  { return uri_;              }
    int                     getHttpVersion() const          { return httpVersion_;      }
    fs::tmpfile const &     getBody() const                 { return body_;             }
    bool                    isSecureHTTP() const            { return secure_;           }

private:
    AddrIn                  localAddress_;      // local address (i.e. the server side)
    AddrIn                  remoteAddress_;     // remote address (i.e. the client side)
    bool                    secure_;            // whether the connection is secured or not (always false: TLS not supported yet)
    HttpVerb                verb_;              // verb (GET, POST, PUT, HEAD, etc.)
    URI                     uri_;               // requested URI
    HttpHeaderMap           headers_;           // request headers
    int                     httpVersion_;       // protocol version
    fs::tmpfile             body_;              // content of the request body

    Result  parseRequestLine(InputStream & s, int timeout, size_t maxsize);
    Result  parseHeaders(InputStream & s, int timeout, size_t maxsize);

    class Body : public OutputStream {          // simple wrapper class to write to a fs::tmpfile as if it were an OutputStream
    public:
        Body(fs::tmpfile & file) : OutputStream(), file_(file)  {                               }
        void write(void const * data, size_t length)            { file_.write(data, length);    }
    private:
        fs::tmpfile & file_;
    };

#ifdef UNIT_TESTING
public:
#else
private:
#endif
    HttpHeaderMap const &   getHeaders() const                  { return headers_;              }
};

extern int const HTTP_VERSION_0_9;              // constant representing HTTP/0.9
extern int const HTTP_VERSION_1_0;              // constant representing HTTP/1.0
extern int const HTTP_VERSION_1_1;              // constant representing HTTP/1.1

//--------------------------------------------------------------

#endif

//========================================================================
