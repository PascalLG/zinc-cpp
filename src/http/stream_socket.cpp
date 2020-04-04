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
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include <cassert>
#include <cstring>
#include <algorithm>

#include "../misc/portability.h"
#include "../misc/logger.h"
#include "stream_socket.h"

using namespace std::literals::chrono_literals;

//--------------------------------------------------------------
// Initialize the WinSock2 library.
//--------------------------------------------------------------

#ifdef _WIN32
static bool initWinSock() {
    static bool winsock_initialized = false;
    if (!winsock_initialized) {
        WSADATA wsa;
        int res = WSAStartup(MAKEWORD(2, 2), &wsa);
        if (res != 0) {
            return false;
        }
        winsock_initialized = true;
    }
    return true;
}
#endif

//========================================================================
// AddrIPv4
//
// Encapsulate an internet address, i.e. an IP address and a TCP port
// number. This class stores and expects all values in host byte order,
// not network byte order.
//========================================================================

//--------------------------------------------------------------
// Build an address from a domain name and a port number.
//--------------------------------------------------------------

AddrIPv4::AddrIPv4(std::string const & name, int port)
  : addr_(0),
    port_(0) {

#ifdef _WIN32
    if (!initWinSock()) {
        return;
    }
#endif

    struct addrinfo hints, * res = nullptr;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(name.c_str(), nullptr, &hints, &res) == 0) {
        if (res != nullptr && port > 0 && port < 65536) {
            auto ipv4 = reinterpret_cast<struct sockaddr_in *>(res->ai_addr);
            addr_ = ntohl(ipv4->sin_addr.s_addr);
            port_ = static_cast<uint16_t>(port);
        }
    }
}

//--------------------------------------------------------------
// Return the IP address as a string.
//--------------------------------------------------------------

std::string AddrIPv4::getAddressString() const {
    struct in_addr in;
    in.s_addr = htonl(addr_);
#ifdef _WIN32
    WCHAR addr[INET_ADDRSTRLEN];
    InetNtopW(AF_INET, &in, addr, INET_ADDRSTRLEN);
    return WideStringToUTF8(addr);
#else
    char addr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &in, addr, INET_ADDRSTRLEN);
    return std::string(addr);
#endif
}

//--------------------------------------------------------------
// Return the port number as a string.
//--------------------------------------------------------------

std::string AddrIPv4::getPortString() const {
    return std::to_string(port_);
}

//--------------------------------------------------------------
// Print the full address to an output stream.
//--------------------------------------------------------------

std::ostream & operator << (std::ostream & os, AddrIPv4 const & rhs) {
    return os << rhs.getAddressString() << ':' << rhs.getPortString();
}

//--------------------------------------------------------------
// Retrieve the name from an IP address. (See below.)
//--------------------------------------------------------------

std::string AddrIPv4::getNameInfo() const {
    static AddrIPv4::Resolver resolver; // Thread-safe as of C++11
    return resolver.resolve(*this);
}

//--------------------------------------------------------------
// Initialize a sockaddr struct with this address.
//--------------------------------------------------------------

void AddrIPv4::fillAddrIn(void * addr, size_t length) const {
    assert(length >= sizeof(struct sockaddr_in));
    memset(addr, 0, length);

    auto addrin = static_cast<struct sockaddr_in *>(addr);
    addrin->sin_family = AF_INET;
    addrin->sin_addr.s_addr = htonl(addr_);
    addrin->sin_port = htons(port_);
}

//========================================================================
// AddrIPv4::Resolver
//
// Helper object to perform thread-safe calls to getnameinfo(). We need
// to implement this pattern because despite POSIX requiring that this
// API be thread-safe, it is actually not on some plateforms.
//========================================================================

//--------------------------------------------------------------
// Does the actual name lookup.
//--------------------------------------------------------------

std::string AddrIPv4::Resolver::resolve(AddrIPv4 const & addr) {
    std::unique_lock<std::mutex> lock(mutex_);

    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(addr.addr_);
    sa.sin_port = htons(addr.port_);

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
    : socket_(INVALID_SOCKET) {
}

//--------------------------------------------------------------
// Construct a StreamSocket object with a socket descriptor.
//--------------------------------------------------------------

StreamSocket::StreamSocket(SOCKET_T socket)
  : socket_(socket) {
    LOG_TRACE("Init socket (fd = " << socket_ << ")");
}

//--------------------------------------------------------------
// Move constructor.
//--------------------------------------------------------------

StreamSocket::StreamSocket(StreamSocket && other)
  : OutputStream(other),
    socket_(other.socket_) {
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
#ifdef _WIN32
    if (!initWinSock()) {
        return false;
    }
#endif
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
// Connect to a server at the specified address.
//--------------------------------------------------------------

bool StreamSocket::connect(AddrIPv4 const & server) {
    if (IS_SOCKET_VALID(socket_)) {
        struct sockaddr_in addr;
        server.fillAddrIn(&addr, sizeof(addr));
        if (::connect(socket_, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) >= 0) {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------
// Bind the socket to a port.
//--------------------------------------------------------------

bool StreamSocket::bind(int port) {
    bool ok = false;

    if (IS_SOCKET_VALID(socket_) && port > 0 && port < 65536) {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));

        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(static_cast<uint16_t>(port));

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

StreamSocket StreamSocket::accept(AddrIPv4 * addr) {
    if (IS_SOCKET_VALID(socket_)) {
        struct sockaddr_in remote;
        socklen_t len = sizeof(remote);
        SOCKET_T client = ::accept(socket_, reinterpret_cast<struct sockaddr *>(&remote), &len);
        if (IS_SOCKET_VALID(client)) {
            if (addr) {
                *addr = AddrIPv4(ntohl(remote.sin_addr.s_addr), ntohs(remote.sin_port));
            }
            return StreamSocket(client);
        }
    }
    return StreamSocket();
}

//--------------------------------------------------------------
// Return the local address of the socket.
//--------------------------------------------------------------

AddrIPv4 StreamSocket::getLocalAddress() {
    struct sockaddr_in local;
    socklen_t len = sizeof(local);
    if (getsockname(socket_, reinterpret_cast<struct sockaddr *>(&local), &len) >= 0) {
        return AddrIPv4(ntohl(local.sin_addr.s_addr), ntohs(local.sin_port));
    }
    return AddrIPv4();
}

//--------------------------------------------------------------
// Wait until data is available for reading.
//--------------------------------------------------------------

int StreamSocket::select(std::chrono::milliseconds timeout) {
    int r = -1;
    if (IS_SOCKET_VALID(socket_)) {
        fd_set readfs;
        FD_ZERO(&readfs);
        FD_SET(socket_, &readfs);
        struct timeval tm;
        tm.tv_sec = static_cast<long>((timeout.count() / 1000));
        tm.tv_usec = static_cast<long>((timeout.count() % 1000) * 1000);
        r = ::select(static_cast<int>(socket_) + 1, &readfs, nullptr, nullptr, &tm);
    }
    return r;
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

size_t StreamSocket::read(void * data, size_t length, std::chrono::milliseconds timeout, bool exact) {
    size_t count = 0;
    while (timeout.count() > 0 && !shutdown_) {
        std::chrono::milliseconds delay = std::min(timeout, 500ms);
        int ret = select(delay);
        if (ret > 0) {
            int r = recv(socket_, static_cast<char *>(data) + count, static_cast<int>(length - count), 0);
            if (r > 0) {
                count += static_cast<size_t>(r);
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

bool StreamSocket::write(void const * data, size_t length) {
    if (length > 0) {
        auto count = static_cast<int>(length);
        if (send(socket_, reinterpret_cast<char const *>(data), count, 0) != count) {
            return false;
        }
    }
    return true;
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
