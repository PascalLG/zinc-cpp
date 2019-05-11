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
#include "http/stream_compress.h"

//--------------------------------------------------------------
// Helper class to write to a string as in a stream.
//--------------------------------------------------------------

#if defined(ZINC_COMPRESSION_DEFLATE) || defined(ZINC_COMPRESSION_GZIP) || defined(ZINC_COMPRESSION_BROTLI)

class HexDump : public OutputStream {
public:
    HexDump() : OutputStream(), empty_(true) {
    }

    void write(void const * data, size_t length) {
        unsigned char const * src = reinterpret_cast<unsigned char const *>(data);
        while (length--) {
            if (empty_) {
                empty_ = false;
            } else {
                buffer_ << ' ';
            }
            char t[8];
            sprintf(t, "%02X", static_cast<int>(*src++));
            buffer_ << t;
        }
    }

    std::string getContent() const {
        return buffer_.str();
    }

private:
    std::ostringstream  buffer_;
    bool                empty_;
};

#endif

//--------------------------------------------------------------
// Test the StreamDeflate class (deflate).
//--------------------------------------------------------------

#if defined(ZINC_COMPRESSION_DEFLATE)

TEST(StreamDeflate, Deflate) {
    HexDump os;
    StreamDeflate transformer(false);
    transformer.setDestination(&os);
    transformer.write("AAA", 3);
    transformer.flush();
    EXPECT_EQ(os.getContent(), "78 DA 73 74 74 04 00 01 89 00 C4");
}

#endif

//--------------------------------------------------------------
// Test the StreamDeflate class (gzip).
//--------------------------------------------------------------

#if defined(ZINC_COMPRESSION_GZIP)

TEST(StreamDeflate, GZip) {
    HexDump os;
    StreamDeflate transformer(true);
    transformer.setDestination(&os);
    transformer.write("AAA", 3);
    transformer.flush();

    // do not compare the 10th byte: it depends on the plateform.
    EXPECT_EQ(os.getContent().substr(0, 26), "1F 8B 08 00 00 00 00 00 02");
    EXPECT_EQ(os.getContent().substr(30, 39), "73 74 74 04 00 A7 31 A0 66 03 00 00 00");
}

#endif

//--------------------------------------------------------------
// Test the StreamBrotli class.
//--------------------------------------------------------------

#if defined(ZINC_COMPRESSION_BROTLI)

TEST(StreamBrotli, Brotli) {
    HexDump os;
    StreamBrotli transformer(BROTLI_MODE_GENERIC, 0);
    transformer.setDestination(&os);
    transformer.write("AAAAAAAAAA", 10);
    transformer.flush();
    EXPECT_EQ(os.getContent(), "8B 04 00 F8 25 82 82 84 00 C0 00");
}

#endif

//========================================================================
