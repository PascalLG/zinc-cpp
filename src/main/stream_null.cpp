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

#include "logger.h"
#include "stream_null.h"

//========================================================================
// StreamNull
//
// Null device. This stream is used for HEAD requests: the processing
// does not change but the data are sent to this null device instead
// of the client socket.
//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

StreamNull::StreamNull()
  : OutputStream() {
    LOG_TRACE("Init StreamNull");
}

//--------------------------------------------------------------
// Destructor.
//--------------------------------------------------------------

StreamNull::~StreamNull() {
    LOG_TRACE("Destroy StreamNull");
}

//--------------------------------------------------------------
// Write a chunk of data on the device.
//--------------------------------------------------------------

void StreamNull::write(void const * data, size_t length) {
    // Do nothing.
}

//========================================================================
