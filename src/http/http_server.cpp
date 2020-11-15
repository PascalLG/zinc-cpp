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
#include "ihttpconfig.h"
#include "uri.h"
#include "resource.h"
#include "http_request.h"
#include "http_response.h"
#include "http_server.h"

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

HttpServer::HttpServer(IHttpConfig & config)
  : config_(config) {
    LOG_TRACE("Init HttpServer");
}

//--------------------------------------------------------------
// Destructor. Stop all the worker threads.
//--------------------------------------------------------------

HttpServer::~HttpServer() {
    LOG_TRACE("Destroy HttpServer");
    pool_.stopAll();
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
        AddrIPv4 remote;
        StreamSocket client = socket_.accept(&remote);
        if (client) {
            LOG_INFO("Accepting connection on socket " << client << " from " << remote);
            auto task = std::make_unique<Connection>(*this, client, client.getLocalAddress(), remote);
            if (!pool_.addTask(std::move(task), config_.getLimitThreads())) {
                LOG_INFO("Maximum number of threads reached, closing connection");
            }
        }
#ifdef ZINC_WEBSOCKET
        websockets_.purge();
#endif
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
// HttpServer::Connection
//
// Represent a connection with a client. A connection can process one or
// several successive requests, depending on the client sending the 
// keep-alive flag or not and/or the protocol version.
//========================================================================

//--------------------------------------------------------------
// Construct a connection object.
//--------------------------------------------------------------

HttpServer::Connection::Connection(HttpServer & server, StreamSocket & socket, AddrIPv4 const & local, AddrIPv4 const & remote)
  : ThreadPool::Task(),
    server_(server),
    socket_(std::move(socket)),
    local_(local),
    remote_(remote) {
    LOG_TRACE("Init HttpServer::Connection");
}

//--------------------------------------------------------------
// Destructor.
//--------------------------------------------------------------

HttpServer::Connection::~Connection() {
    LOG_TRACE("Destroy HttpServer::Connection");
}

//--------------------------------------------------------------
// Process requests.
//--------------------------------------------------------------

void HttpServer::Connection::run(int /* no */) {
    bool keepalive;
    do {
        // Parse the request and resolve which local resource
        // to transmit.

        std::shared_ptr<Resource> body;
        HttpRequest request(local_, remote_, false);
        HttpRequest::Result r = request.parse(socket_, server_.config_.getTimeout(), static_cast<size_t>(server_.config_.getLimitRequestLine()), static_cast<size_t>(server_.config_.getLimitRequestHeaders()), static_cast<size_t>(server_.config_.getLimitRequestBody()));
        if (r.isAborted()) {
            break;
        } else if (r.isError()) {
            keepalive = false;
            body = server_.config_.makeErrorPage(r.getHttpStatus());
        } else {
#ifdef ZINC_WEBSOCKET
            HttpRequest::Result ws = request.isWebSocketUpgrade();
            if (ws.isError()) {
                keepalive = false;
                body = server_.config_.makeErrorPage(ws.getHttpStatus());
            } else if (ws.isOK()) {
                LOG_INFO("Switching protocol on socket " << socket_);
                server_.websockets_.add(server_.config_, std::move(socket_)).handshake(request);
                return;
            } else {
#endif
                keepalive = request.shouldKeepAlive();
                if (request.getVerb().isOneOf(HttpVerb::Get | HttpVerb::Head | HttpVerb::Post | HttpVerb::Put | HttpVerb::Delete)) {
                    body = server_.config_.resolve(request.getURI());
                } else {
                    body = server_.config_.makeErrorPage(405);
                }
#ifdef ZINC_WEBSOCKET
            }
#endif
        }

        // Build and transmit a response.

        HttpResponse response(server_.config_, request, socket_, keepalive ? HttpResponse::Connection::KeepAlive : HttpResponse::Connection::Close);
        LOG_INFO_SEND("Replying: " << body->getDescription());
        body->transmit(response, request);

        // Loop until the client or the server request a
        // connection close.

    } while (keepalive);
    LOG_INFO("Closing connection on socket " << socket_);
}

//========================================================================
