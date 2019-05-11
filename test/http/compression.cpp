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
#include "misc/logger.h"
#include "http/compression.h"
#include "http/stream_compress.h"

//--------------------------------------------------------------
// Test the getCompressionName function.
//--------------------------------------------------------------

TEST(Compression, getName) {
    EXPECT_EQ(getCompressionName(compression::none),            "");
#if defined(ZINC_COMPRESSION_GZIP)
    EXPECT_EQ(getCompressionName(compression::gzip),            "gzip");
#endif
#if defined(ZINC_COMPRESSION_DEFLATE)
    EXPECT_EQ(getCompressionName(compression::deflate),         "deflate");
#endif
#if defined(ZINC_COMPRESSION_BROTLI)
    EXPECT_EQ(getCompressionName(compression::brotli_generic),  "br");
    EXPECT_EQ(getCompressionName(compression::brotli_text),     "br");
    EXPECT_EQ(getCompressionName(compression::brotli_font),     "br");
#endif
}

//--------------------------------------------------------------
// Test the parseAcceptedEncodings function.
//--------------------------------------------------------------

TEST(Compression, parseAcceptedEncodings) {
    EXPECT_EQ(parseAcceptedEncodings(""),                   compression::set());
    EXPECT_EQ(parseAcceptedEncodings("foo"),                compression::set());
#if defined(ZINC_COMPRESSION_GZIP)
    EXPECT_EQ(parseAcceptedEncodings("gzip"),               compression::set({ compression::gzip }));
#endif
#if defined(ZINC_COMPRESSION_DEFLATE)
    EXPECT_EQ(parseAcceptedEncodings("deflate"),            compression::set({ compression::deflate }));
#endif
#if defined(ZINC_COMPRESSION_BROTLI)
    EXPECT_EQ(parseAcceptedEncodings("br"),                 compression::set({ compression::brotli_generic, compression::brotli_text, compression::brotli_font }));
#endif
#if defined(ZINC_COMPRESSION_GZIP) && defined(ZINC_COMPRESSION_DEFLATE)
    EXPECT_EQ(parseAcceptedEncodings("gzip,deflate"),       compression::set({ compression::gzip, compression::deflate }));
    EXPECT_EQ(parseAcceptedEncodings("gzip, deflate"),      compression::set({ compression::gzip, compression::deflate }));
    EXPECT_EQ(parseAcceptedEncodings("gzip, foo, deflate"), compression::set({ compression::gzip, compression::deflate }));
    EXPECT_EQ(parseAcceptedEncodings(" gzip , deflate "),   compression::set({ compression::gzip, compression::deflate }));
#endif
}

//--------------------------------------------------------------
// Test the selectCompressionMode function.
//--------------------------------------------------------------

TEST(Compression, selectCompressionMode) {
#if defined(ZINC_COMPRESSION_GZIP) && defined(ZINC_COMPRESSION_BROTLI)
    EXPECT_EQ(selectCompressionMode(parseAcceptedEncodings("gzip, br"),      "unknown/unknown"), compression::none);
    EXPECT_EQ(selectCompressionMode(parseAcceptedEncodings("gzip, br"),      "text/plain"),      compression::brotli_text);
#endif
#if defined(ZINC_COMPRESSION_GZIP) && defined(ZINC_COMPRESSION_DEFLATE)
    EXPECT_EQ(selectCompressionMode(parseAcceptedEncodings("gzip, deflate"), "text/plain"),      compression::gzip);
#endif
#if defined(ZINC_COMPRESSION_DEFLATE)
    EXPECT_EQ(selectCompressionMode(parseAcceptedEncodings("deflate"),       "text/plain"),      compression::deflate);
#endif
    EXPECT_EQ(selectCompressionMode(parseAcceptedEncodings(""),              "text/plain"),      compression::none);
}

//--------------------------------------------------------------
// Test the makeStreamTransformer function.
//--------------------------------------------------------------

TEST(Compression, makeStreamTransformer) {
    auto f = [] (compression::mode mode) {
        std::unique_ptr<OutputStream> s = makeStreamTransformer(mode, 0);
        OutputStream & r = *s.get();
        return typeid(r).hash_code();
    };
    logger::setLevel(logger::error, false);
#if defined(ZINC_COMPRESSION_GZIP)
    EXPECT_EQ(f(compression::gzip), typeid(StreamDeflate).hash_code());
#endif
#if defined(ZINC_COMPRESSION_DEFLATE)
    EXPECT_EQ(f(compression::deflate), typeid(StreamDeflate).hash_code());
#endif
#if defined(ZINC_COMPRESSION_BROTLI)
    EXPECT_EQ(f(compression::brotli_generic), typeid(StreamBrotli).hash_code());
    EXPECT_EQ(f(compression::brotli_text), typeid(StreamBrotli).hash_code());
    EXPECT_EQ(f(compression::brotli_font), typeid(StreamBrotli).hash_code());
#endif
}

//========================================================================
