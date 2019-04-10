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
#include "uri.h"

//--------------------------------------------------------------
// Test the URI class (simple path, no query string).
//--------------------------------------------------------------

TEST(URI, SimplePath) {
    URI u;

    EXPECT_TRUE(u.parse("/"));
    EXPECT_EQ(u.getPath(), "/");
    EXPECT_EQ(u.getQuery(), "");
    EXPECT_EQ(u.getRequestURI(false), "/");
    EXPECT_EQ(u.getRequestURI(true), "/");
    EXPECT_EQ(u.getArguments().size(), 0);

    EXPECT_TRUE(u.parse("/index.html"));
    EXPECT_EQ(u.getPath(), "/index.html");
    EXPECT_EQ(u.getQuery(), "");
    EXPECT_EQ(u.getArguments().size(), 0);
}

//--------------------------------------------------------------
// Test the URI class (one argument in the query string).
//--------------------------------------------------------------

TEST(URI, QueryOneArg) {
    URI u;

    EXPECT_TRUE(u.parse("/?a=123"));
    EXPECT_EQ(u.getPath(), "/");
    EXPECT_EQ(u.getQuery(), "a=123");
    EXPECT_EQ(u.getRequestURI(false), "/?a=123");
    EXPECT_EQ(u.getRequestURI(true), "/?a=123");
    EXPECT_EQ(u.getArguments().size(), 1);
    EXPECT_EQ(u.getArguments().find("a")->second, "123");

    EXPECT_TRUE(u.parse("/index.html?a=xyz"));
    EXPECT_EQ(u.getPath(), "/index.html");
    EXPECT_EQ(u.getQuery(), "a=xyz");
    EXPECT_EQ(u.getRequestURI(false), "/index.html?a=xyz");
    EXPECT_EQ(u.getRequestURI(true), "/index.html/?a=xyz");
    EXPECT_EQ(u.getArguments().size(), 1);
    EXPECT_EQ(u.getArguments().find("a")->second, "xyz");
}

//--------------------------------------------------------------
// Test the URI class (three arguments in the query string).
//--------------------------------------------------------------

TEST(URI, QueryMultiArg) {
    URI u;

    EXPECT_TRUE(u.parse("/index.html?a=123;b=x%24;c=this+is+a+test"));
    EXPECT_EQ(u.getPath(), "/index.html");
    EXPECT_EQ(u.getQuery(), "a=123;b=x%24;c=this+is+a+test");
    EXPECT_EQ(u.getRequestURI(false), "/index.html?a=123;b=x%24;c=this+is+a+test");
    EXPECT_EQ(u.getRequestURI(true), "/index.html/?a=123;b=x%24;c=this+is+a+test");
    EXPECT_EQ(u.getArguments().size(), 3);
    EXPECT_EQ(u.getArguments().find("a")->second, "123");
    EXPECT_EQ(u.getArguments().find("b")->second, "x$");
    EXPECT_EQ(u.getArguments().find("c")->second, "this is a test");
}

//--------------------------------------------------------------
// Test the URI class (simple argument).
//--------------------------------------------------------------

TEST(URI, SimpleArg) {
    URI u;

    EXPECT_TRUE(u.parse("/?lock"));
    EXPECT_EQ(u.getPath(), "/");
    EXPECT_EQ(u.getQuery(), "lock");
    EXPECT_EQ(u.getRequestURI(false), "/?lock");
    EXPECT_EQ(u.getRequestURI(true), "/?lock");
    EXPECT_EQ(u.getArguments().size(), 1);
    EXPECT_EQ(u.getArguments().find("lock")->second, "");

    EXPECT_TRUE(u.parse("/index.html?lock"));
    EXPECT_EQ(u.getPath(), "/index.html");
    EXPECT_EQ(u.getQuery(), "lock");
    EXPECT_EQ(u.getRequestURI(false), "/index.html?lock");
    EXPECT_EQ(u.getRequestURI(true), "/index.html/?lock");
    EXPECT_EQ(u.getArguments().size(), 1);
    EXPECT_EQ(u.getArguments().find("lock")->second, "");
}

//--------------------------------------------------------------
// Test the URI class (ill-formed URI).
//--------------------------------------------------------------

TEST(URI, IllFormed) {
    URI u;

    EXPECT_FALSE(u.parse("/index.html??a=1"));
    EXPECT_FALSE(u.parse("/index.html?a=1?b=2"));
    EXPECT_FALSE(u.parse("/inde%3.html?a=123"));
    EXPECT_FALSE(u.parse("/index.html?a%2h=123"));
    EXPECT_FALSE(u.parse("/index.html?a=%0"));
}

//========================================================================
