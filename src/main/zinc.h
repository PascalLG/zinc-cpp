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

#ifndef __ZINC_H__
#define __ZINC_H__

#include "../http/ihttpconfig.h"
#include "configuration.h"

//--------------------------------------------------------------
// Zinc server configuration
//--------------------------------------------------------------

class Zinc : public IHttpConfig {
public:
    Configuration & getConfiguration()                              { return configuration_;                           }
    static Zinc &   getInstance();

    int                     getListeningPort() override             { return configuration_.getListeningPort();        }
    int                     getLimitThreads() override              { return configuration_.getLimitThreads();         }
    int                     getLimitRequestLine() override          { return configuration_.getLimitRequestLine();     }
    int                     getLimitRequestHeaders() override       { return configuration_.getLimitRequestHeaders();  }
    int                     getLimitRequestBody() override          { return configuration_.getLimitRequestBody();     }
    std::chrono::seconds    getTimeout() override                   { return configuration_.getTimeout();              }
    bool                    isCompressionEnabled() override         { return configuration_.isCompressionEnabled();    }
    std::string             getVersionString() override;

    std::shared_ptr<Resource>   resolve(URI const & uri) override;
    std::shared_ptr<Resource>   makeErrorPage(HttpStatus status) override;
    bool                        acceptConnection(AddrIPv4 const & remote) override;

#ifdef ZINC_WEBSOCKET
    void    handleMessage(WebSocket::Connection & socket, WebSocket::Frame & frame) override;
#endif

private:
    Zinc();

    Configuration configuration_; 
};

//--------------------------------------------------------------

#endif

//========================================================================
