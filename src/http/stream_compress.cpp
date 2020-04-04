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
#include "stream_compress.h"

//========================================================================
// StreamDeflate
//
// Stream transformer that compress data on the fly using the deflate or
// gzip algorithm.
//========================================================================

#if defined(ZINC_COMPRESSION_GZIP) || defined(ZINC_COMPRESSION_DEFLATE)

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

StreamDeflate::StreamDeflate(bool gzip)
  : OutputStream() {
    LOG_TRACE("Init StreamDeflate (gzip = " << gzip << ")");

    int windowbits = 15;
    if (gzip) {
        windowbits |= 16;
    }

    memset(&state_, 0, sizeof(state_));
    deflateInit2(&state_, 9, Z_DEFLATED, windowbits, 8, Z_DEFAULT_STRATEGY);
}

//--------------------------------------------------------------
// Destructor.
//--------------------------------------------------------------

StreamDeflate::~StreamDeflate() {
    LOG_TRACE("Destroy StreamDeflate");
    deflateEnd(&state_);
}

//--------------------------------------------------------------
// Write a chunk of data.
//--------------------------------------------------------------

bool StreamDeflate::write(void const * data, size_t length) {
    state_.avail_in = length;
    state_.next_in = reinterpret_cast<unsigned char const *>(data);
    compress(Z_NO_FLUSH);
    return true;
}

//--------------------------------------------------------------
// Flush the stream.
//--------------------------------------------------------------

bool StreamDeflate::flush() {
    state_.avail_in = 0;
    compress(Z_FINISH);
    return getDestination()->flush();
}

//--------------------------------------------------------------
// Compress the current chunk of data and write the output
// to the destination stream.
//--------------------------------------------------------------

void StreamDeflate::compress(int flush) {
    do {
        unsigned char buffer[1024];
        state_.next_out = buffer;
        state_.avail_out = sizeof(buffer);

        deflate(&state_, flush);
        getDestination()->write(buffer, sizeof(buffer) - state_.avail_out);

        LOG_TRACE("Deflate encode: " << sizeof(buffer) - state_.avail_out << " bytes");
    } while (state_.avail_out == 0);
}

//--------------------------------------------------------------

#endif // defined(ZINC_COMPRESSION_GZIP) || defined(ZINC_COMPRESSION_DEFLATE)

//========================================================================
// StreamBrotli
//
// Stream transformer that compress data on the fly using the Brotli
// algorithm.
//========================================================================

#if defined(ZINC_COMPRESSION_BROTLI)

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

StreamBrotli::StreamBrotli(BrotliEncoderMode mode, long length)
    : OutputStream(),
      state_(nullptr) {

    LOG_TRACE("Init StreamBrotli (mode = " << mode << ", length = " << length << ")");
    state_ = BrotliEncoderCreateInstance(0, 0, nullptr);
    BrotliEncoderSetParameter(state_, BROTLI_PARAM_MODE, mode);
    BrotliEncoderSetParameter(state_, BROTLI_PARAM_QUALITY, BROTLI_MAX_QUALITY);
    BrotliEncoderSetParameter(state_, BROTLI_PARAM_SIZE_HINT, std::max(0l, length));
}

//--------------------------------------------------------------
// Destructor.
//--------------------------------------------------------------

StreamBrotli::~StreamBrotli() {
    LOG_TRACE("Destroy StreamBrotli");
    BrotliEncoderDestroyInstance(state_);
}

//--------------------------------------------------------------
// Write a chunk of data.
//--------------------------------------------------------------

bool StreamBrotli::write(void const * data, size_t length) {
    unsigned char const * next_in = reinterpret_cast<unsigned char const *>(data);
    do {
        unsigned char buffer[1024];
        unsigned char * next_out = buffer;
        size_t avail_out = sizeof(buffer);

        BrotliEncoderCompressStream(state_, BROTLI_OPERATION_PROCESS, &length, &next_in, &avail_out, &next_out, nullptr);
        getDestination()->write(buffer, sizeof(buffer) - avail_out);

        LOG_TRACE("Brotli encode: " << sizeof(buffer) - avail_out << " bytes");
    } while (length);
    return true;
}

//--------------------------------------------------------------
// Flush the stream.
//--------------------------------------------------------------

bool StreamBrotli::flush() {
    do {
        unsigned char buffer[1024];
        unsigned char * next_out = buffer;
        unsigned char const * next_in = nullptr;
        size_t avail_out = sizeof(buffer), avail_in = 0;

        BrotliEncoderCompressStream(state_, BROTLI_OPERATION_FLUSH, &avail_in, &next_in, &avail_out, &next_out, nullptr);
        getDestination()->write(buffer, sizeof(buffer) - avail_out);

        LOG_TRACE("encode " << sizeof(buffer) - avail_out << " bytes");
    } while (BrotliEncoderHasMoreOutput(state_));

    return getDestination()->flush();
}

//--------------------------------------------------------------

#endif // defined(ZINC_COMPRESSION_BROTLI)

//========================================================================
