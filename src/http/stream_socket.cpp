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

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include <cstring>
#include <algorithm>

#include "../misc/portability.h"
#include "../misc/logger.h"
#include "stream_socket.h"

//========================================================================
// AddrIn
//
// Encapsulate an internet address, i.e. an IP address and a TCP port
// number. This class stores and expects all values in host byte order,
// not network byte order.
//========================================================================

//--------------------------------------------------------------
// Return the IP address as a string.
//--------------------------------------------------------------

std::string AddrIn::getAddress() const {
#ifdef _WIN32
    WCHAR addr[INET_ADDRSTRLEN];
    struct in_addr in = { htonl(addr_) };
    InetNtopW(AF_INET, &in, addr, INET_ADDRSTRLEN);
    return WideStringToUTF8(addr);
#else
    char addr[INET_ADDRSTRLEN];
    struct in_addr in = { htonl(addr_) };
    inet_ntop(AF_INET, &in, addr, INET_ADDRSTRLEN);
    return std::string(addr);
#endif
}

//--------------------------------------------------------------
// Return the port number as a string.
//--------------------------------------------------------------

std::string AddrIn::getPort() const {
    return std::to_string(port_);
}

//--------------------------------------------------------------
// Print the full address to an output stream.
//--------------------------------------------------------------

std::ostream & operator << (std::ostream & os, AddrIn const & rhs) {
    return os << rhs.getAddress() << ':' << rhs.getPort();
}

//--------------------------------------------------------------
// Retrieve the name from an IP address. (See below.)
//--------------------------------------------------------------

std::string AddrIn::getNameInfo() const {
    static AddrIn::Resolver resolver; // Thread-safe as of C++11
    return resolver.resolve(*this);
}

//========================================================================
// AddrIn::Resolver
//
// Helper object to perform thread-safe calls to getnameinfo(). We need
// to implement this pattern because despite POSIX requiring that this
// API be thread-safe, it is actually not on some plateforms.
//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

AddrIn::Resolver::Resolver()
  : mutex_() {
}

//--------------------------------------------------------------
// Does the actual name lookup.
//--------------------------------------------------------------

std::string AddrIn::Resolver::resolve(AddrIn const & addr) {
    std::unique_lock<std::mutex> lock(mutex_);

    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htonl(addr.port_);
    sa.sin_addr.s_addr = htons(addr.addr_);

    char host[1028];
    getnameinfo(reinterpret_cast<struct sockaddr *>(&sa), sizeof(sa), host, sizeof(host), nullptr, 0, 0);

    return std::string(host);
}

//========================================================================
// StreamSocket
//
// Encapsulate a socket into a class providing streaming facilities. Also
// provides portability to other modules in using sockets. Since the 
// underlying BSD socket cannot be duplicated, this class explicitly
// forbids copy constructor and copy assignment.
//========================================================================

//--------------------------------------------------------------
// Default constructor.
//--------------------------------------------------------------

StreamSocket::StreamSocket()
    : OutputStream(), socket_(INVALID_SOCKET) {
}

//--------------------------------------------------------------
// Construct a StreamSocket object with a socket descriptor.
//--------------------------------------------------------------

StreamSocket::StreamSocket(SOCKET_T socket)
  : OutputStream(), socket_(socket) {
    LOG_TRACE("Init socket (fd = " << socket_ << ")");
}

//--------------------------------------------------------------
// Move constructor.
//--------------------------------------------------------------

StreamSocket::StreamSocket(StreamSocket && other)
  : OutputStream(other), socket_(other.socket_) {
    other.socket_ = INVALID_SOCKET;
}

//--------------------------------------------------------------
// Destructor.
//--------------------------------------------------------------

StreamSocket::~StreamSocket() {
    if (IS_SOCKET_VALID(socket_)) {
        LOG_TRACE("Destroy socket (fd = " << socket_ << ")");
        closesocket(socket_);
    }
}

//--------------------------------------------------------------
// Move assignment.
//--------------------------------------------------------------

StreamSocket & StreamSocket::operator = (StreamSocket && other) {
    setDestination(other.getDestination());
    std::swap(socket_, other.socket_);
    return *this;
}

//--------------------------------------------------------------
// Create the underlying socket with appropriate options.
//--------------------------------------------------------------

bool StreamSocket::create() {
    SOCKET_T s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (IS_SOCKET_VALID(s)) {
        int optval = 1;
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char const *>(&optval), sizeof(optval));

        if (IS_SOCKET_VALID(socket_)) {
            closesocket(socket_);
        }
        socket_ = s;

        return true;
    }
    return false;
}

//--------------------------------------------------------------
// Bind the socket to a port.
//--------------------------------------------------------------

bool StreamSocket::bind(uint16_t port) {
    bool ok = false;

    if (IS_SOCKET_VALID(socket_)) {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));

        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port);

        if (::bind(socket_, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) >= 0) {
            ok = true;
        }
    }

    return ok;
}

//--------------------------------------------------------------
// Put the socket in listen mode.
//--------------------------------------------------------------

bool StreamSocket::listen() {
    return IS_SOCKET_VALID(socket_) && ::listen(socket_, SOMAXCONN) >= 0;
}

//--------------------------------------------------------------
// Accept an incoming connection, optionally returning the
// remote address of the socket.
//--------------------------------------------------------------

StreamSocket StreamSocket::accept(AddrIn * addr) {
    if (IS_SOCKET_VALID(socket_)) {
        struct sockaddr_in remote;
        socklen_t len = sizeof(remote);
        SOCKET_T client = ::accept(socket_, reinterpret_cast<struct sockaddr *>(&remote), &len);
        if (IS_SOCKET_VALID(client)) {
            if (addr) {
                *addr = AddrIn(ntohl(remote.sin_addr.s_addr), ntohs(remote.sin_port));
            }
            return StreamSocket(client);
        }
    }
    return StreamSocket();
}

//--------------------------------------------------------------
// Return the local address of the socket.
//--------------------------------------------------------------

AddrIn StreamSocket::getLocalAddress() {
    struct sockaddr_in local;
    socklen_t len = sizeof(local);
    if (getsockname(socket_, reinterpret_cast<struct sockaddr *>(&local), &len) >= 0) {
        return AddrIn(ntohl(local.sin_addr.s_addr), ntohs(local.sin_port));
    }
    return AddrIn();
}

//--------------------------------------------------------------
// Close the underlying socket.
//--------------------------------------------------------------

void StreamSocket::close() {
    if (IS_SOCKET_VALID(socket_)) {
        closesocket(socket_);
        socket_ = INVALID_SOCKET;
    }
}

//--------------------------------------------------------------
// Read a chunk of data from the socket. Return the number of
// bytes read, or zero if an error occurred. If the exact
// parameter is true, does not return until the exact number
// of requested bytes are read, otherwise return when at least
// one byte is read.
//--------------------------------------------------------------

size_t StreamSocket::read(void * data, size_t length, int timeout, bool exact) {
    size_t count = 0;
    while (timeout > 0 && !shutdown_) {
        fd_set readfs;
        FD_ZERO(&readfs);
        FD_SET(socket_, &readfs);

        struct timeval tm;
        int delay = std::min(timeout, 500);
        tm.tv_sec = (delay / 1000);
        tm.tv_usec = (delay % 1000) * 1000;

        int ret = select(socket_ + 1, &readfs, NULL, NULL, &tm);
        if (ret > 0) {
            int r = recv(socket_, static_cast<char *>(data) + count, length - count, 0);
            if (r > 0) {
                count += r;
                if (count >= length || !exact) {
                    return count;
                }
            } else {
                break;
            }
        }

        timeout -= delay;
    }
    LOG_TRACE("Socket timeout (fd = " << socket_ << ")");
    return 0;
}

//--------------------------------------------------------------
// Write a chunk of data on the socket.
//--------------------------------------------------------------

void StreamSocket::write(void const * data, size_t length) {
    if (length > 0) {
        send(socket_, reinterpret_cast<char const *>(data), length, 0);
    }
}

//--------------------------------------------------------------
// Shutdown. If set to true, abort ASAP all reading operations
// on all existing sockets.
//--------------------------------------------------------------

bool StreamSocket::shutdown_ = false;

void StreamSocket::shutdown(bool shutdown) {
    LOG_TRACE("Socket shutdown = " << shutdown);
    shutdown_ = shutdown;
}

//========================================================================
