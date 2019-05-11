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

#ifndef __STREAM_H__
#define __STREAM_H__

#include <functional>
#include <string>

#include "http_header.h"

//--------------------------------------------------------------
// Input stream.
//--------------------------------------------------------------

class InputStream {
public:
    int             readByte(int timeout);
    virtual size_t  read(void * data, size_t length, int timeout, bool exact) = 0;
};

//--------------------------------------------------------------
// Output stream.
//--------------------------------------------------------------

class OutputStream {
public:
    OutputStream() : destination_(nullptr)                                      {                               }
    OutputStream(OutputStream const & other) : destination_(other.destination_) {                               }
    virtual ~OutputStream()                                                     {                               }

    void            setDestination(OutputStream * destination)                  { destination_ = destination;   }
    OutputStream  * getDestination() const                                      { return destination_;    		}

    virtual void    write(void const * data, size_t length) = 0;
    virtual void    flush();

    void            emitEol();
    void            emitHeader(HttpHeader const & header, std::string const & value);
    void            emitPage(char const * text);
    void            emitPage(char const * text, std::function<std::string(std::string const &)> const & fields);

private:
    OutputStream * destination_;
};

//--------------------------------------------------------------

#endif

//========================================================================
