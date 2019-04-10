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

#include "config.h"
#include "logger.h"
#include "http_request.h"
#include "http_response.h"
#include "http_server.h"
#include "uri.h"
#include "resource.h"
#include "resource_error_page.h"

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

HttpConnection::HttpConnection(StreamSocket & socket, AddrIn const & local, AddrIn const & remote)
  : socket_(std::move(socket)),
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
        HttpRequest::Result r = request.parse(socket_);
        if (r.isAborted()) {
            return;
        } else if (r.isError()) {
            keepalive = false;
            body = std::make_shared<ResourceErrorPage>(r.getHttpStatus());
        } else {
            keepalive = request.shouldKeepAlive();
            if (request.getVerb().isOneOf(HttpVerb::GET | HttpVerb::HEAD | HttpVerb::POST | HttpVerb::PUT | HttpVerb::DELETE)) {
                body = request.getURI().resolve();
            } else {
                body = std::make_shared<ResourceErrorPage>(405);
            }
        }

        // Build and transmit a response.

        HttpResponse response(request, socket_, keepalive);
        LOG_INFO_SEND("Replying: " << body->getDescription());
        body->transmit(response, request);

        // Loop until the client or the server request a
        // connection close.

    } while (keepalive);
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

HttpServer::HttpServer()
  : socket_(),
    stop_(false),
    workers_(),
    connections_(),
    condition_(),
    mutex_() {

    LOG_TRACE("Init HttpServer");
    int limit = Configuration::getInstance().getLimitThreads();
    for (int i = 1; i <= limit; i++) {
        workers_.emplace_back([this, i] {
            logger::registerWorkerThread(i);
            LOG_TRACE("Start thread #" << i);
            for( ; ; ) {
                std::unique_ptr<HttpConnection> connection;
                {
                    std::unique_lock<std::mutex> lock(this->mutex_);
                    this->condition_.wait(lock, [this] {
                        return this->stop_ || !this->connections_.empty();
                    });
                    if(this->stop_ && this->connections_.empty()) {
                        LOG_TRACE("Stop thread #" << i);
                        return;
                    }
                    connection = std::move(this->connections_.front());
                    this->connections_.pop();
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

    {
        std::unique_lock<std::mutex> lock(mutex_);
        stop_ = true;
    }

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

    if (!socket_.bind(Configuration::getInstance().getListeningPort())) {
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
            LOG_TRACE("Accepting connection on socket " << client << " from " << remote);
            std::unique_lock<std::mutex> lock(mutex_);
            connections_.emplace(std::make_unique<HttpConnection>(client, client.getLocalAddress(), remote));
            condition_.notify_one();
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
