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
#include "http/http_header.h"

//--------------------------------------------------------------
// Test the HttpHeader class creation and comparison.
//--------------------------------------------------------------

TEST(HttpHeader, Creation) {
    HttpHeader x1(HttpHeader::ContentType);
    HttpHeader x2("CoNtEnT-tYpE");

    HttpHeader y1(HttpHeader::TransferEncoding);
    HttpHeader y2("transfer-encoding");

    HttpHeader z1("FooBar");
    HttpHeader z2("foobar");

    EXPECT_EQ(x1, x2);
    EXPECT_EQ(x2, x1);
    EXPECT_EQ(y1, y2);
    EXPECT_EQ(y2, y1);
    EXPECT_EQ(z1, z2);
    EXPECT_EQ(z2, z1);

    EXPECT_EQ(x1.getFieldName(),  "Content-Type");
    EXPECT_EQ(x2.getFieldName(),  "Content-Type");
    EXPECT_EQ(y1.getFieldName(),  "Transfer-Encoding");
    EXPECT_EQ(y2.getFieldName(),  "Transfer-Encoding");
    EXPECT_EQ(z1.getFieldName(),  "FooBar");
    EXPECT_EQ(z2.getFieldName(),  "foobar");

    EXPECT_EQ(x1.getHash(), x2.getHash());
    EXPECT_EQ(y1.getHash(), y2.getHash());
    EXPECT_EQ(z1.getHash(), z2.getHash());

    EXPECT_NE(x1.getHash(), y1.getHash());
    EXPECT_NE(x1.getHash(), z1.getHash());
    EXPECT_NE(y1.getHash(), z1.getHash());
}

//--------------------------------------------------------------
// Test the HttpHeader class in an STL container.
//--------------------------------------------------------------

TEST(HttpHeader, Container) {
    std::unordered_map<HttpHeader, std::string> map;
    map.emplace(HttpHeader::ContentType, "text/plain");
    map.emplace(HttpHeader::ContentLength, "1234");

    EXPECT_EQ(map.find(HttpHeader::Accept), map.end());
    EXPECT_EQ(map.find(HttpHeader::ContentType)->second, "text/plain");
    EXPECT_EQ(map.find(HttpHeader::ContentLength)->second, "1234");
}

//========================================================================
