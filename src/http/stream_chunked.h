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

#ifndef __STREAM_CHUNKED_H__
#define __STREAM_CHUNKED_H__

#include <functional>

#include "stream.h"

//--------------------------------------------------------------
// Stream transformer for chunked Transfer-Encoding.
//--------------------------------------------------------------

#define CHUNK_MAXSIZE		4096

class StreamChunked : public OutputStream {
public:
    StreamChunked(std::function<void(long)> const & emit, size_t maxlen = CHUNK_MAXSIZE);
    ~StreamChunked();

    void    write(void const * data, size_t length) override;
    void    flush() override;

private:
    std::function<void(long)>   emitHeaders_;           // callback to emit the HTTP headers
    bool                        headersSent_;           // flag to remember if HTTP headers are already sent
    char                        currentChunk_[CHUNK_MAXSIZE];    // current chunk of data
    size_t		                maxChunkLength_;		// maximum size of a chunk
    size_t                      chunkLength_;           // number of bytes in the current chunk

    void    encodeChunk();
};

//--------------------------------------------------------------

#endif

//========================================================================
