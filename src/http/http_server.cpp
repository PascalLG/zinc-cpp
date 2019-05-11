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

#include "../misc/logger.h"
#include "iconfig.h"
#include "uri.h"
#include "resource.h"
#include "http_request.h"
#include "http_response.h"
#include "http_server.h"

//========================================================================
// HttpConnection
//
// Represent a connection with a client. A connection can process one or
// several successive requests, depending on the client sending the 
// keep-alive flag or not and/or the protocol version.
//========================================================================

//--------------------------------------------------------------
// Construct an HTTP connection object.
//--------------------------------------------------------------

HttpConnection::HttpConnection(IConfig & config, StreamSocket & socket, AddrIn const & local, AddrIn const & remote)
  : config_(config),
    socket_(std::move(socket)),
    local_(local),
    remote_(remote) {
    LOG_TRACE("Init HttpConnection");
}

//--------------------------------------------------------------
// Destructor.
//--------------------------------------------------------------

HttpConnection::~HttpConnection() {
    LOG_TRACE("Destroy HttpConnection");
}

//--------------------------------------------------------------
// Process requests.
//--------------------------------------------------------------

void HttpConnection::process() {
    bool keepalive;
    do {
        // Parse the request and resolve which local resource
        // to transmit.

        std::shared_ptr<Resource> body;
        HttpRequest request(local_, remote_, false);
        HttpRequest::Result r = request.parse(config_, socket_);
        if (r.isAborted()) {
            break;
        } else if (r.isError()) {
            keepalive = false;
            body = config_.makeErrorPage(r.getHttpStatus());
        } else {
            keepalive = request.shouldKeepAlive();
            if (request.getVerb().isOneOf(HttpVerb::Get | HttpVerb::Head | HttpVerb::Post | HttpVerb::Put | HttpVerb::Delete)) {
                body = config_.resolve(request.getURI());
            } else {
                body = config_.makeErrorPage(405);
            }
        }

        // Build and transmit a response.

        HttpResponse response(config_, request, socket_, keepalive);
        LOG_INFO_SEND("Replying: " << body->getDescription());
        body->transmit(response, request);

        // Loop until the client or the server request a
        // connection close.

    } while (keepalive);
    LOG_INFO("Closing connection on socket " << socket_);
}

//========================================================================
// HttpServer
//
// Implement the HTTP server. A socket is bound to the specified port to
// accept incoming requests and a pool of threads is used to process
// connections as they arrive. 
//========================================================================

//--------------------------------------------------------------
// Constructor. Start the pool of worker threads.
//--------------------------------------------------------------

HttpServer::HttpServer(IConfig & config)
  : config_(config),
    socket_(),
    stop_(false),
    workers_(),
    connections_(),
    condition_(),
    mutex_(),
    idle_(0) {

    LOG_TRACE("Init HttpServer");
    int limit = config_.getLimitThreads();
    for (int i = 1; i <= limit; i++) {
        workers_.emplace_back([this, i] {
            logger::registerWorkerThread(i);
            LOG_TRACE("Start thread #" << i);
            for( ; ; ) {
                std::unique_ptr<HttpConnection> connection;
                {
                    std::unique_lock<std::mutex> lock(this->mutex_);
                    this->idle_++;
                    this->condition_.wait(lock, [this] {
                        return this->stop_ || !this->connections_.empty();
                    });
                    if(this->stop_) {
                        LOG_TRACE("Stop thread #" << i);
                        return;
                    }
                    connection = std::move(this->connections_.front());
                    this->connections_.pop();
                    this->idle_--;
                }
                connection->process();
            }
        });
    }
}

//--------------------------------------------------------------
// Destructor. Stop all the worker threads.
//--------------------------------------------------------------

HttpServer::~HttpServer() {
    LOG_TRACE("Destroy HttpServer");
    stop_ = true;
    condition_.notify_all();
    for (std::thread & w: workers_) {
        w.join();
    }
}

//--------------------------------------------------------------
// Start listening. This function does not return until the server
// is halted by calling stop().
//--------------------------------------------------------------

int HttpServer::startup() {

    // Create and configure the server socket.

    if (!socket_.create()) {
        LOG_ERROR("Unable to create server socket");
        return EXIT_FAILURE;
    }

    if (!socket_.bind(config_.getListeningPort())) {
        LOG_ERROR("Unable to bind server socket");
        return EXIT_FAILURE;
    }

    if (!socket_.listen()) {
        LOG_ERROR("Unable to start listening on server socket");
        return EXIT_FAILURE;
    }

    StreamSocket::shutdown(false);
    LOG_INFO("Server is up and listening");

    while (socket_) {
        AddrIn remote;
        StreamSocket client = socket_.accept(&remote);
        if (client) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (idle_ > 0) {
                LOG_INFO("Accepting connection on socket " << client << " from " << remote);
                connections_.emplace(std::make_unique<HttpConnection>(config_, client, client.getLocalAddress(), remote));
                condition_.notify_one();
            } else {
                LOG_INFO("Refusing connection on socket " << client << " from " << remote);
                client.close();
            }
        }
    }

    LOG_INFO("Server is going down");
    return EXIT_SUCCESS;
}

//--------------------------------------------------------------
// Stop the server and cause the startup() function to return.
// Does nothing if the server is not running.
//--------------------------------------------------------------

void HttpServer::stop() {
    StreamSocket::shutdown(true);
    socket_.close();
}

//========================================================================
