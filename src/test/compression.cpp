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
#include "compression.h"

//--------------------------------------------------------------
// Test the getCompressionName function.
//--------------------------------------------------------------

TEST(Compression, getName) {
    EXPECT_EQ(getCompressionName(compression::none),            "");
    EXPECT_EQ(getCompressionName(compression::gzip),            "gzip");
    EXPECT_EQ(getCompressionName(compression::deflate),         "deflate");
    EXPECT_EQ(getCompressionName(compression::brotli_generic),  "br");
    EXPECT_EQ(getCompressionName(compression::brotli_text),     "br");
    EXPECT_EQ(getCompressionName(compression::brotli_font),     "br");
}

//--------------------------------------------------------------
// Test the parseAcceptedEncodings function.
//--------------------------------------------------------------

TEST(Compression, parseAcceptedEncodings) {
    EXPECT_EQ(parseAcceptedEncodings(""),                   compression::set());
    EXPECT_EQ(parseAcceptedEncodings("gzip"),               compression::set({ compression::gzip }));
    EXPECT_EQ(parseAcceptedEncodings("deflate"),            compression::set({ compression::deflate }));
    EXPECT_EQ(parseAcceptedEncodings("br"),                 compression::set({ compression::brotli_generic, compression::brotli_text, compression::brotli_font }));
    EXPECT_EQ(parseAcceptedEncodings("foo"),                compression::set());
    EXPECT_EQ(parseAcceptedEncodings("gzip,deflate"),       compression::set({ compression::gzip, compression::deflate }));
    EXPECT_EQ(parseAcceptedEncodings("gzip, deflate"),      compression::set({ compression::gzip, compression::deflate }));
    EXPECT_EQ(parseAcceptedEncodings("gzip, foo, deflate"), compression::set({ compression::gzip, compression::deflate }));
    EXPECT_EQ(parseAcceptedEncodings(" gzip , deflate "),   compression::set({ compression::gzip, compression::deflate }));
}

//--------------------------------------------------------------
// Test the selectCompressionMode function.
//--------------------------------------------------------------

TEST(Compression, selectCompressionMode) {
    EXPECT_EQ(selectCompressionMode(parseAcceptedEncodings("gzip, br"),      "unknown/unknown"), compression::none);
    EXPECT_EQ(selectCompressionMode(parseAcceptedEncodings("gzip, br"),      "text/plain"),      compression::brotli_text);
    EXPECT_EQ(selectCompressionMode(parseAcceptedEncodings("gzip, deflate"), "text/plain"),      compression::gzip);
    EXPECT_EQ(selectCompressionMode(parseAcceptedEncodings("deflate"),       "text/plain"),      compression::deflate);
    EXPECT_EQ(selectCompressionMode(parseAcceptedEncodings(""),              "text/plain"),      compression::none);
}

//========================================================================
