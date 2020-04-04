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

#ifndef STREAM_SOCKET_H
#define STREAM_SOCKET_H

#include <ostream>
#include <mutex>
#include <string>

#include "../misc/portability.h"
#include "stream.h"

//--------------------------------------------------------------
// Internet Address.
//--------------------------------------------------------------

class AddrIPv4 {
public:
    AddrIPv4() : addr_(0), port_(0)                                                 {   }
    AddrIPv4(uint32_t addr, uint16_t port) : addr_(addr), port_(port)               {   }
    AddrIPv4(AddrIPv4 const & other) : addr_(other.addr_), port_(other.port_)       {   }
    AddrIPv4(std::string const & name, int port);

    AddrIPv4 & operator = (AddrIPv4 const & other)                                  { addr_ = other.addr_; port_ = other.port_; return *this;   }
    operator bool() const                                                           { return addr_ != 0 && port_ != 0;                          }

    std::string     getAddressString() const;
    std::string     getPortString() const;
    std::string     getNameInfo() const;

    void            fillAddrIn(void * addrin, size_t length) const;

    friend std::ostream & operator << (std::ostream & os, AddrIPv4 const & rhs);
    friend bool           operator == (AddrIPv4 const & lhs, AddrIPv4 const & rhs)  { return lhs.addr_ == rhs.addr_ && lhs.port_ == rhs.port_; }

private:
    uint32_t    addr_;
    uint16_t    port_;

    class Resolver {
    public:
        Resolver() = default;
        std::string resolve(AddrIPv4 const & addr);

    private:
        std::mutex  mutex_;
    };
};

//--------------------------------------------------------------
// Socket.
//--------------------------------------------------------------

class StreamSocket : public OutputStream, public InputStream {
public:
    StreamSocket();
    StreamSocket(StreamSocket const &)                   = delete;
    StreamSocket(StreamSocket && other);
    virtual ~StreamSocket() override;

    StreamSocket &  operator = (StreamSocket const &)    = delete;
    StreamSocket &  operator = (StreamSocket && other);

    operator bool() const                                                               { return IS_SOCKET_VALID(socket_);  }
    friend std::ostream & operator << (std::ostream & os, StreamSocket const & rhs)     { return os << rhs.socket_;         }

    bool            create();
    bool            connect(AddrIPv4 const & server);
    bool            bind(int port);
    bool            listen();
    StreamSocket    accept(AddrIPv4 * addr);
    AddrIPv4        getLocalAddress();
    int             select(std::chrono::milliseconds timeout);
    void            close();

    size_t          read(void * data, size_t length, std::chrono::milliseconds timeout, bool exact) override;
    bool            write(void const * data, size_t length) override;

    static void     shutdown(bool shutdown);

private:
    StreamSocket(SOCKET_T socket);

    SOCKET_T        socket_;    // BSD socket
    static bool     shutdown_;  // server is shuting down
};

//--------------------------------------------------------------

#endif

//========================================================================
