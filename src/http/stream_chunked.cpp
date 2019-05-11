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

#include <cstring>
#include <algorithm>

#include "../misc/logger.h"
#include "stream_chunked.h"

//========================================================================
// StreamChunked
//
// Stream transformer for chunked Transfer-Encoding. Also serves as a
// temporary buffer: if the whole data is shorter than this class internal
// buffer, transfer is not chunked. To achieve this, the HttpResponse
// class defers the sending of the HTTP headers and provides this class
// a callback function to emit them.
//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

StreamChunked::StreamChunked(std::function<void(long)> const & emit, size_t maxlen)
    : OutputStream(),
      emitHeaders_(emit),
      headersSent_(false),
      currentChunk_{0},
      maxChunkLength_(maxlen),
      chunkLength_(0) {

    LOG_TRACE("Init StreamChunked (buffer size = " << maxChunkLength_ << ")");
}

//--------------------------------------------------------------
// Destructor.
//--------------------------------------------------------------

StreamChunked::~StreamChunked() {
    LOG_TRACE("Destroy StreamChunked");
}

//--------------------------------------------------------------
// Write a chunk of data. As soon as enough data are bufferized,
// transfer begins in chunked mode.
//--------------------------------------------------------------

void StreamChunked::write(void const * data, size_t length) {
    char const * ptr = reinterpret_cast<char const *>(data);
    size_t ndx = 0;

    while (ndx < length) {
        if (chunkLength_ == maxChunkLength_) {
            if (!headersSent_) {
                emitHeaders_(-1);
                headersSent_ = true;
            }
            encodeChunk();
        }
        size_t size = std::min(maxChunkLength_ - chunkLength_, length - ndx);
        memcpy(currentChunk_ + chunkLength_, ptr + ndx, size);
        chunkLength_ += size;
        ndx += size;
    }
}

//--------------------------------------------------------------
// Flush the stream. If the headers were already sent, just
// send the last chunk and a terminator. Otherwise, send the
// whole buffer as a single, not-chunked transfer.
//--------------------------------------------------------------

void StreamChunked::flush() {
    if (headersSent_) {
        if (chunkLength_ > 0) {
            encodeChunk();
        }
        encodeChunk(); // terminator
    } else {
        emitHeaders_(chunkLength_);
        getDestination()->write(currentChunk_, chunkLength_);
    }
    getDestination()->flush();
}

//--------------------------------------------------------------
// Encode the (possibly empty) current chunk of data. Refer to
// RFC 7230 section 4.1 for more information.
//--------------------------------------------------------------

void StreamChunked::encodeChunk() {
    char size[16];
    sprintf(size, "%lx", static_cast<unsigned long>(chunkLength_));
    LOG_TRACE("=> chunk of 0x" << size << " bytes");

    getDestination()->write(size, strlen(size));
    getDestination()->emitEol();
    getDestination()->write(currentChunk_, chunkLength_);
    getDestination()->emitEol();

    chunkLength_ = 0;
}

//========================================================================
