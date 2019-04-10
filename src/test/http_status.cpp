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
#include "http_status.h"

//--------------------------------------------------------------
// Test the HttpStatus class creation and assignment.
//--------------------------------------------------------------

TEST(HttpStatus, Creation) {
    HttpStatus x1(200);
    HttpStatus x2(x1);
    HttpStatus x3 = x2;

    HttpStatus y1(403);
    HttpStatus y2(y1);
    HttpStatus y3 = y2;

    EXPECT_EQ(x1.getStatusCode(), 200);
    EXPECT_EQ(x2.getStatusCode(), 200);
    EXPECT_EQ(x3.getStatusCode(), 200);

    EXPECT_EQ(y1.getStatusCode(), 403);
    EXPECT_EQ(y2.getStatusCode(), 403);
    EXPECT_EQ(y3.getStatusCode(), 403);
}

//--------------------------------------------------------------
// Test the HttpStatus class comparison operators.
//--------------------------------------------------------------

TEST(HttpStatus, Comparison) {
    HttpStatus x1(301);
    HttpStatus x2(301);
    HttpStatus y1(404);
    HttpStatus y2(404);

    EXPECT_EQ(x1, x2);
    EXPECT_EQ(x2, x1);
    EXPECT_EQ(y1, y2);
    EXPECT_EQ(y2, y1);

    EXPECT_NE(x1, y1);
    EXPECT_NE(y1, x1);
    EXPECT_NE(x2, y2);
    EXPECT_NE(y2, x2);
}

//--------------------------------------------------------------
// Test the HttpStatus.getStatusString() function.
//--------------------------------------------------------------

TEST(HttpStatus, getStatusString) {
    EXPECT_EQ(HttpStatus(100).getStatusString(), "Continue");
    EXPECT_EQ(HttpStatus(200).getStatusString(), "OK");
    EXPECT_EQ(HttpStatus(418).getStatusString(), "I’m a teapot");
    EXPECT_EQ(HttpStatus(500).getStatusString(), "Internal Server Error");
    EXPECT_EQ(HttpStatus(511).getStatusString(), "Network authentication required");
    EXPECT_EQ(HttpStatus(999).getStatusString(), "");
}

//========================================================================
