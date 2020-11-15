//========================================================================
// Zinc - Web Server
// Copyright (c) 2020, Pascal Levy
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

#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#ifdef ZINC_WEBSOCKET

#include <string>
#include <vector>
#include <thread>
#include <list>

#include "../misc/prng.h"
#include "ihttpconfig.h"
#include "stream_socket.h"
#include "http_request.h"

namespace WebSocket {

class Connection;
class ConnectionList;

//--------------------------------------------------------------
// WebSocket frame.
//--------------------------------------------------------------

class Frame {
public:
    enum Opcode {
        Text    = 0x01,
        Binary  = 0x02,
        Close   = 0x08,
        Ping    = 0x09,
        Pong    = 0x0A,
    };

    Frame();

    bool receive(InputStream & input, std::chrono::milliseconds timeout);
    bool send(OutputStream & output, iprng & prng, bool masked) const;

    void setTextMessage(char const * message);
    void setBinaryMessage(void const * message, size_t length);
    void setCloseMessage(int code);

    Opcode                          getMessageType() const      { return opcode_;                                           }
    std::string                     getTextMessage() const      { return std::string(payload_.cbegin(), payload_.cend());   }
    std::vector<uint8_t> const &    getBinaryMessage() const    { return payload_;                                          }
    int                             getCloseMessage() const;

private:
    std::vector<uint8_t>    payload_;
    Opcode                  opcode_;
};

//--------------------------------------------------------------
// WebSocket connection.
//--------------------------------------------------------------

class Connection {
public:
    Connection(ConnectionList & parent, IHttpConfig & config, StreamSocket socket);
    ~Connection();

    void    handshake(HttpRequest const & request);
    void    sendMessage(Frame const & message);

    bool    isConnected() const { return socket_;   }

private:
    ConnectionList &    parent_;
    IHttpConfig &       config_;
    StreamSocket        socket_;
    std::thread         listener_;

    void    listen();
};

//--------------------------------------------------------------
// WebSocket connection list.
//--------------------------------------------------------------

class ConnectionList {
public:
    ConnectionList() = default;

    Connection &    add(IHttpConfig & config, StreamSocket socket);
    void            broadcast(Frame const & frame);
    void            purge();

private:
    std::list<Connection>   list_;
    std::mutex              mutex_;
};

//--------------------------------------------------------------
// WebSocket helper functions.
//--------------------------------------------------------------

std::string MakeNonce(iprng & prng);
std::string TransformNonce(std::string const & nonce);

//--------------------------------------------------------------

}

#endif
#endif

//========================================================================
