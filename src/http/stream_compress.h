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

#ifndef STREAM_COMPRESS_H
#define STREAM_COMPRESS_H

#if defined(ZINC_COMPRESSION_GZIP) || defined(ZINC_COMPRESSION_DEFLATE)
#define ZLIB_CONST 1
#include <zlib.h>
#endif

#if defined(ZINC_COMPRESSION_BROTLI)
#include <brotli/encode.h>
#endif

#include "stream.h"

//--------------------------------------------------------------
// Stream transformer for deflate/gzip content encoding.
//--------------------------------------------------------------

#if defined(ZINC_COMPRESSION_GZIP) || defined(ZINC_COMPRESSION_DEFLATE)
class StreamDeflate : public OutputStream {
public:
    StreamDeflate(bool gzip);
    ~StreamDeflate() override;

    bool write(void const * data, size_t length) override;
    bool flush() override;

private:
    z_stream state_;

    void compress(int flush);
};
#endif

//--------------------------------------------------------------
// Stream transformer for brotli content encoding.
//--------------------------------------------------------------

#if defined(ZINC_COMPRESSION_BROTLI)
class StreamBrotli : public OutputStream {
public:
    StreamBrotli(BrotliEncoderMode mode, long length);
    ~StreamBrotli() override;

    bool write(void const * data, size_t length) override;
    bool flush() override;

private:
    BrotliEncoderState * state_;
};
#endif

//--------------------------------------------------------------

#endif

//========================================================================
