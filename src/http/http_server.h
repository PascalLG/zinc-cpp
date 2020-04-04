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

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <list>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "ihttpconfig.h"
#include "stream_socket.h"
#include "websocket.h"

//--------------------------------------------------------------
// An HTTP connection with a client.
//--------------------------------------------------------------

class HttpConnection {
public:
    HttpConnection(IHttpConfig & config, StreamSocket & socket, AddrIPv4 const & local, AddrIPv4 const & remote);
    ~HttpConnection();

#ifdef ZINC_WEBSOCKET
    void process(WebSocket::ConnectionList & websockets);
#else
    void process();
#endif

private:
    IHttpConfig &   config_;    // server configuration
    StreamSocket    socket_;    // connection with the client
    AddrIPv4        local_;     // local address (i.e. the server)
    AddrIPv4        remote_;    // remote address (i.e. the client)
};

//--------------------------------------------------------------
// The HTTP server.
//--------------------------------------------------------------

class HttpServer {
public:
    HttpServer(IHttpConfig & config);
    ~HttpServer();

    int     startup();
    void    stop();

#ifdef ZINC_WEBSOCKET
    void    broadcast(WebSocket::Frame const & frame)       { websockets_.broadcast(frame); }
#endif

private:
    IHttpConfig &                               config_;            // server configuration
    StreamSocket                                socket_;            // server socket
    bool                                        stop_;              // if true, shutdown in progress
    std::vector<std::thread>                    workers_;           // list of worker threads
    std::queue<std::unique_ptr<HttpConnection>> connections_;       // queue of active connections waiting to be processed
    std::condition_variable                     condition_;         // thread synchronization
    std::mutex                                  mutex_;             // thread synchronization
    int                                         idle_;              // number of idle threads
#ifdef ZINC_WEBSOCKET
    WebSocket::ConnectionList                   websockets_;        // active websocket connections
#endif
};

//--------------------------------------------------------------

#endif

//========================================================================
