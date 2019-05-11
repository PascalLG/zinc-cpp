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

#ifndef __ICONFIG_H__
#define __ICONFIG_H__

#include <memory>

#include "uri.h"
#include "http_status.h"
#include "resource.h"

//--------------------------------------------------------------

class IConfig {
public:
    virtual std::shared_ptr<Resource>   resolve(URI const & uri)            = 0;
    virtual std::shared_ptr<Resource>   makeErrorPage(HttpStatus status)    = 0;

    virtual int                         getListeningPort()                  = 0;
    virtual int                         getLimitThreads()                   = 0;
    virtual int                         getLimitRequestLine()               = 0;
    virtual int                         getLimitRequestHeaders()            = 0;
    virtual int                         getLimitRequestBody()               = 0;
    virtual int                         getTimeout()                        = 0;
    virtual bool                        isCompressionEnabled()              = 0;
    virtual std::string					getVersionString()					= 0;
};

//--------------------------------------------------------------

#endif

//========================================================================
