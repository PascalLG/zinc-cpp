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
#include "misc/blob.h"

//--------------------------------------------------------------
// Test the blob class.
//--------------------------------------------------------------

TEST(Blob, Creation) {
    blob f;
    EXPECT_TRUE(f.write("ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26));
    EXPECT_TRUE(f.write("0123456789", 10));
    EXPECT_EQ(f.getSize(), 36);
    EXPECT_TRUE(f.write("abcdefghijklmnopqrstuvwxyz", 26));
    EXPECT_EQ(f.getSize(), 62);

    std::vector<uint8_t> content = f.readAll();
    content.push_back(0);
    EXPECT_EQ(content.size(), 63);
    EXPECT_STREQ(reinterpret_cast<char const *>(content.data()), "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz");
}

//========================================================================
