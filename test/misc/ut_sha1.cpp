//========================================================================
// Zinc - Unit Testing
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

#include "gtest/gtest.h"
#include "misc/sha1.h"

//--------------------------------------------------------------
// Test the SHA-1 function.
//--------------------------------------------------------------

TEST(Sha1, Global) {
    digest::sha1 hash;

    auto print = [](digest::sha1 & sha) {
        std::array<uint8_t, 20> digest;
        sha.finalize(digest);
        char buffer[44];
        for (size_t i = 0; i < 20; i++) {
            sprintf(buffer + 2 * i, "%02x", digest[i]);
        }
        return std::string(buffer);
    };

    auto test = [&print](digest::sha1 & sha, char const * text) {
        sha.init();
        sha.update(text, strlen(text));
        return print(sha);
    };

    EXPECT_EQ("da39a3ee5e6b4b0d3255bfef95601890afd80709", test(hash, ""));
    EXPECT_EQ("a9993e364706816aba3e25717850c26c9cd0d89d", test(hash, "abc"));
    EXPECT_EQ("32d10c7b8cf96570ca04ce37f2a19d84240d3a89", test(hash, "abcdefghijklmnopqrstuvwxyz"));
    EXPECT_EQ("52c4ac98d05f285b2d9b086703fe0463a3045475", test(hash, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzab"));
    EXPECT_EQ("a617d006d1ca12671785098a19a87fe58443bde9", test(hash, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabc"));
    EXPECT_EQ("4ad5bb7ae3c4024768d364b77c52128ea3cffebe", test(hash, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcd"));
    EXPECT_EQ("e1b3b34da0f7b299090824d9aa81fff6711a79ad", test(hash, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcde"));
    EXPECT_EQ("281990516def979bcf61c25b05e068e0ffab4827", test(hash, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdef"));
    EXPECT_EQ("64696b2babc6b85a09e5630d03c4975cc015a729", test(hash, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefg"));
    EXPECT_EQ("6d8c64099fafda87af096c9e8185de824d3c54ad", test(hash, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefgh"));
    EXPECT_EQ("d343477267541bfba0f9b05c64187cd8812a2f1b", test(hash, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghi"));
    EXPECT_EQ("3ba53d6cb1408a0cb35520428a5916aefca1500e", test(hash, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghij"));
    EXPECT_EQ("fc8a5ab77259625085ead3ec96515b3b8d933fad", test(hash, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijk"));
    EXPECT_EQ("93249d4c2f8903ebf41ac358473148ae6ddd7042", test(hash, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijkl"));
    EXPECT_EQ("cf2a63cc308225cf07b498d2309a01dd0df52f67", test(hash, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklm"));
    EXPECT_EQ("3698f9beb7cdf5b1e1ce786672fb234143d4df89", test(hash, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwx"));

    hash.init();
    for (size_t i = 0; i < 10000; i++) {
        hash.update("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 100);
    }
    EXPECT_EQ(print(hash), "34aa973cd4c4daa4f61eeb2bdbad27316534016f");

    hash.init();
    for (size_t i = 0; i < 1000; i++) {
        char buf[24];
        int nr = sprintf(buf, "+%d+ABCDEF+%c", static_cast<int>(i), static_cast<char>((i & 127) + 32));
        hash.update(buf, nr);
    }
    EXPECT_EQ(print(hash), "4b0e2a878f6c229781792d9f15a0c90e02a1aaf5");
}

//========================================================================
