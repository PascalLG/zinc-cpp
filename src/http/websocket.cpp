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

#ifdef ZINC_WEBSOCKET

#include "../misc/logger.h"
#include "../misc/base64.h"
#include "../misc/sha1.h"

#include "http_response.h"
#include "websocket.h"

using namespace std::literals::chrono_literals;

//========================================================================
// WebSocket::Frame
//
// Encapsulate a WebSocket frame. See RFC 6455 for more information. Also
// provide methods to send and receive frames on a stream.
//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

WebSocket::Frame::Frame()
  : opcode_(Close) {
}

//--------------------------------------------------------------
// Receive and decode a frame.
//--------------------------------------------------------------

bool WebSocket::Frame::receive(InputStream & input, std::chrono::milliseconds timeout) {
    uint8_t buffer[256];
    static_assert((sizeof(buffer) % 4) == 0, "Buffer size must be a multiple of 4");

    if (!input.read(buffer, 2, timeout, true)) {
        return false;
    }
    opcode_ = static_cast<Opcode>(buffer[0] & 0x0F);
    bool masked = (buffer[1] & 0x80) != 0;
    size_t size = buffer[1] & 0x7F;

    if (size == 126) {
        if (!input.read(buffer, 2, timeout, true)) {
            return false;
        }
        size = (static_cast<size_t>(buffer[0]) << 8)
             | static_cast<size_t>(buffer[1]);
    } else if (size == 127) {
        if (!input.read(buffer, 8, timeout, true)) {
            return false;
        }
        size = (static_cast<size_t>(buffer[4]) << 24)
             | (static_cast<size_t>(buffer[5]) << 16)
             | (static_cast<size_t>(buffer[6]) << 8)
             | static_cast<size_t>(buffer[7]);
    }

    uint8_t mask[4];
    if (masked) {
        if (!input.read(mask, 4, timeout, true)) {
            return false;
        }
    } else {
        memset(mask, 0, sizeof(mask));
    }

    payload_.clear();
    while (size) {
        size_t len = std::min(size, sizeof(buffer));
        if (!input.read(buffer, len, timeout, true)) {
            return false;
        }
        for (size_t i = 0; i < len; i++) {
            buffer[i] ^= mask[i & 3];
        }
        payload_.insert(payload_.end(), buffer, buffer + len);
        size -= len;
    }

    return true;
}

//--------------------------------------------------------------
// Send a frame.
//--------------------------------------------------------------

bool WebSocket::Frame::send(OutputStream & output, iprng & prng, bool masked) const {
    uint8_t buffer[256];
    static_assert((sizeof(buffer) % 4) == 0, "Buffer size must be a multiple of 4");

    size_t size = payload_.size();
    size_t offset = 1;

    buffer[0] = static_cast<uint8_t>(0x80 + opcode_);
    buffer[1] = masked ? 0x80 : 0x00;

    if (size <= 125) {
        buffer[offset++] |= size;
    } else if (size <= 65535) {
        buffer[offset++] |= 126;
        buffer[offset++] = static_cast<uint8_t>(size >> 8);
        buffer[offset++] = static_cast<uint8_t>(size);
    } else {
        buffer[offset++] |= 127;
        buffer[offset++] = 0;   // ignore higher bits, frames bigger than 2^31 bytes are unlikely to happen
        buffer[offset++] = 0;
        buffer[offset++] = 0;
        buffer[offset++] = 0;
        buffer[offset++] = static_cast<uint8_t>(size >> 24);
        buffer[offset++] = static_cast<uint8_t>(size >> 16);
        buffer[offset++] = static_cast<uint8_t>(size >> 8);
        buffer[offset++] = static_cast<uint8_t>(size);
    }

    uint8_t mask[4];
    if (masked) {
        uint32_t v = prng.next();
        for (size_t i = 0; i < 4; i++) {
            mask[i] = static_cast<uint8_t>(v);
            buffer[offset++] = mask[i];
            v >>= 8;
        }
    } else {
        memset(mask, 0, sizeof(mask));
    }

    if (!output.write(buffer, offset)) {
        return false;
    }

    uint8_t const * source = payload_.data();
    while (size) {
        size_t len = std::min(size, sizeof(buffer));
        for (size_t i = 0; i < len; i++) {
            buffer[i] = source[i] ^ mask[i & 3];
        }
        if (!output.write(buffer, len)) {
            return false;
        }
        source += len;
        size -= len;
    }
    return true;
}

//--------------------------------------------------------------
// Initialize the frame content with a text message.
//--------------------------------------------------------------

void WebSocket::Frame::setTextMessage(std::string const & message) {
    payload_.assign(message.cbegin(), message.cend());
    opcode_ = Text;
}

//--------------------------------------------------------------
// Initialize the frame content with a binary message.
//--------------------------------------------------------------

void WebSocket::Frame::setBinaryMessage(void const * message, size_t length) {
    auto src = static_cast<uint8_t const *>(message);
    payload_.assign(src, src + length);
    opcode_ = Binary;
}

//--------------------------------------------------------------
// Initialize the frame content with a close message.
//--------------------------------------------------------------

void WebSocket::Frame::setCloseMessage(int code) {
    std::array<uint8_t, 2> buffer = {
        static_cast<uint8_t>(code >> 8),
        static_cast<uint8_t>(code),
    };
    payload_.assign(buffer.cbegin(), buffer.cend());
    opcode_ = Close;
}

//--------------------------------------------------------------
// Decode the frame content as a close message.
//--------------------------------------------------------------

int WebSocket::Frame::getCloseMessage() const {
    int result = 0;
    if (payload_.size() >= 2) {
        int high = payload_[0];
        int low = payload_[1];
        result = (high << 8) | low;
    }
    return result;
}

//========================================================================
// WebSocket::Connection
//
// Implement the server side of a WebSocket connection.
//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

WebSocket::Connection::Connection(ConnectionList & parent, IHttpConfig & config, StreamSocket socket)
  : parent_(parent),
    config_(config),
    socket_(std::move(socket)),
    listener_([this] () { listen(); }) {

    LOG_TRACE("Init WebSocket::Connection");
}

//--------------------------------------------------------------
// Destructor.
//--------------------------------------------------------------

WebSocket::Connection::~Connection() {
    socket_.close();
    listener_.join();
    LOG_TRACE("Destroy WebSocket::Connection");
}

//--------------------------------------------------------------
// Send a handshake response to a WebSocket upgrade request.
//--------------------------------------------------------------

void WebSocket::Connection::handshake(HttpRequest const & request) {
    HttpResponse response(config_, request, socket_, HttpResponse::Connection::Upgrade);
    response.setHttpStatus(101);
    response.emitHeader(HttpHeader::Upgrade, "websocket");
    response.emitHeader(HttpHeader::SecWebSocketAccept, TransformNonce(request.getHeaderValue(HttpHeader::SecWebSocketKey)));
    response.emitEol();
    response.flush();
}

//--------------------------------------------------------------
// Send a message.
//--------------------------------------------------------------

void WebSocket::Connection::sendMessage(WebSocket::Frame const & message) {
    message.send(socket_, prng::instance(), false);
}

//--------------------------------------------------------------
// Listener thread.
//--------------------------------------------------------------

void WebSocket::Connection::listen() {
    for ( ; ; ) {
        int r = socket_.select(60s);
        if (r > 0) {
            WebSocket::Frame frame;
            if (!frame.receive(socket_, 10s)) {
                socket_.close();
                return;
            }
            config_.handleMessage(*this, frame);
        }
    }
}

//========================================================================
// WebSocket::ConnectionList
//
// Implement a thread safe list of connections.
//========================================================================

//--------------------------------------------------------------
// Create a new connection and add it to the list.
//--------------------------------------------------------------

WebSocket::Connection & WebSocket::ConnectionList::add(IHttpConfig & config, StreamSocket socket) {
    std::lock_guard<std::mutex> lock(mutex_);
    list_.emplace_back(*this, config, std::move(socket));
    return list_.back();
}

//--------------------------------------------------------------
// Remove a connection from the list.
//--------------------------------------------------------------

void WebSocket::ConnectionList::purge() {
    std::lock_guard<std::mutex> lock(mutex_);
    list_.remove_if([] (Connection const & item) { return !item.isConnected(); });
}

//--------------------------------------------------------------
// Broadcast a message to all connections in the list.
//--------------------------------------------------------------

void WebSocket::ConnectionList::broadcast(Frame const & frame) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (Connection & c: list_) {
        c.sendMessage(frame);
    }
}

//========================================================================
// WebSocket helper functions.
//========================================================================

//--------------------------------------------------------------
// Generate a random nonce suitable for WebSocket handshake.
//--------------------------------------------------------------

std::string WebSocket::MakeNonce(iprng & prng) {
    uint8_t nonce[16];
    for (size_t i = 0; i < sizeof(nonce) / sizeof(nonce[0]); i++) {
        nonce[i] = static_cast<uint8_t>(prng.next());
    }
    return base64::encode(nonce, sizeof(nonce));
}

//--------------------------------------------------------------
// Transform a nonce for WebSocket handshake.
//--------------------------------------------------------------

std::string WebSocket::TransformNonce(std::string const & nonce) {
    static char const * guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    digest::sha1 sha;
    std::array<uint8_t, 20> digest;
    std::string accept = nonce + guid;
    sha.update(accept.data(), accept.size());
    sha.finalize(digest);

    return base64::encode(digest.data(), digest.size());
}

//--------------------------------------------------------------

#endif

//========================================================================
