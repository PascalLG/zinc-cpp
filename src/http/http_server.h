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

#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "stream_socket.h"

class IConfig;

//--------------------------------------------------------------
// An HTTP connection with a client.
//--------------------------------------------------------------

class HttpConnection {
public:
    HttpConnection(IConfig & config, StreamSocket & socket, AddrIn const & local, AddrIn const & remote);
    ~HttpConnection();

    void process();

private:
    IConfig &       config_;    // server configuration
    StreamSocket    socket_;    // connection with the client
    AddrIn          local_;     // local address (i.e. the server)
    AddrIn          remote_;    // remote address (i.e. the client)
};

//--------------------------------------------------------------
// The HTTP server.
//--------------------------------------------------------------

class HttpServer {
public:
	HttpServer(IConfig & config);
	~HttpServer();

	int		startup();
	void	stop();

private:
    IConfig &                                   config_;            // server configuration
    StreamSocket                                socket_;            // server socket
    bool                                        stop_;              // if true, shutdown in progress
    std::vector<std::thread>                    workers_;           // list of worker threads
    std::queue<std::unique_ptr<HttpConnection>> connections_;       // queue of active connections waiting to be processed
    std::condition_variable                     condition_;         // thread synchronization
    std::mutex                                  mutex_;             // thread synchronization
    int                                         idle_;              // number of idle threads
};

//--------------------------------------------------------------

#endif

//========================================================================
