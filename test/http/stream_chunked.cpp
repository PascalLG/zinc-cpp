//========================================================================
// Zinc - Unit Testing
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

#include "gtest/gtest.h"
#include "http/stream_chunked.h"

//--------------------------------------------------------------
// Helper class to write to a string as in a stream.
//--------------------------------------------------------------

class BinDump : public OutputStream {
public:
    BinDump() : OutputStream() {
    }

    void write(void const * data, size_t length) {
        buffer_.write(reinterpret_cast<char const *>(data), length);
    }

    void emitHeaders(long length) {
        if (length >= 0) {
            buffer_ << "Length: " << length;
        } else {
            buffer_ << "Chunked";
        }
        buffer_ << "|";
    }

    std::string getContent() const {
        return buffer_.str();
    }

private:
    std::ostringstream  buffer_;
};


//--------------------------------------------------------------
// Test the StreamChunked class (case 1).
//--------------------------------------------------------------

TEST(StreamChunked, Case1) {
    BinDump os;
    StreamChunked transformer([&os](long length) { os.emitHeaders(length); }, 10);
    transformer.setDestination(&os);
    transformer.flush();
    EXPECT_EQ(os.getContent(), "Length: 0|");
}

//--------------------------------------------------------------
// Test the StreamChunked class (case 2).
//--------------------------------------------------------------

TEST(StreamChunked, Case2) {
    BinDump os;
    StreamChunked transformer([&os](long length) { os.emitHeaders(length); }, 10);
    transformer.setDestination(&os);
    transformer.write("ABC", 3);
    transformer.flush();
    EXPECT_EQ(os.getContent(), "Length: 3|ABC");
}

//--------------------------------------------------------------
// Test the StreamChunked class (case 3).
//--------------------------------------------------------------

TEST(StreamChunked, Case3) {
    BinDump os;
    StreamChunked transformer([&os](long length) { os.emitHeaders(length); }, 10);
    transformer.setDestination(&os);
    transformer.write("abcdefghij", 10);
    transformer.flush();
    EXPECT_EQ(os.getContent(), "Length: 10|abcdefghij");
}

//--------------------------------------------------------------
// Test the StreamChunked class (case 4).
//--------------------------------------------------------------

TEST(StreamChunked, Case4) {
    BinDump os;
    StreamChunked transformer([&os](long length) { os.emitHeaders(length); }, 10);
    transformer.setDestination(&os);
    transformer.write("abcdefghijk", 11);
    transformer.flush();
    EXPECT_EQ(os.getContent(), "Chunked|a\r\nabcdefghij\r\n1\r\nk\r\n0\r\n\r\n");
}

//--------------------------------------------------------------
// Test the StreamChunked class (case 5).
//--------------------------------------------------------------

TEST(StreamChunked, Case5) {
    BinDump os;
    StreamChunked transformer([&os](long length) { os.emitHeaders(length); }, 16);
    transformer.setDestination(&os);
    transformer.write("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52);
    transformer.flush();
    EXPECT_EQ(os.getContent(), "Chunked|10\r\nABCDEFGHIJKLMNOP\r\n10\r\nQRSTUVWXYZabcdef\r\n10\r\nghijklmnopqrstuv\r\n4\r\nwxyz\r\n0\r\n\r\n");
}

//========================================================================
