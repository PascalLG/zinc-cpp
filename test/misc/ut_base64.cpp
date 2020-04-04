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
#include "misc/base64.h"

//--------------------------------------------------------------
// Test the base64::encode() function.
//--------------------------------------------------------------

TEST(Base64, Encode) {
    auto t = [] (char const * text) -> std::string {
        return base64::encode(text, strlen(text));
    };

    EXPECT_EQ(t(""),       ""         );
    EXPECT_EQ(t("f"),      "Zg=="     );
    EXPECT_EQ(t("fo"),     "Zm8="     );
    EXPECT_EQ(t("foo"),    "Zm9v"     );
    EXPECT_EQ(t("foob"),   "Zm9vYg==" );
    EXPECT_EQ(t("fooba"),  "Zm9vYmE=" );
    EXPECT_EQ(t("foobar"), "Zm9vYmFy" );

    uint8_t tmp[256];
    for (size_t i = 0; i < 256; i++) {
        tmp[i] = static_cast<uint8_t>(i);
    }
    EXPECT_EQ(base64::encode(tmp, 256), "AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKissLS4vMDEyMzQ1Njc4OTo7PD0+P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWltcXV5fYGFiY2RlZmdoaWprbG1ub3BxcnN0dXZ3eHl6e3x9fn+AgYKDhIWGh4iJiouMjY6PkJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrq+wsbKztLW2t7i5uru8vb6/wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7/P3+/w==");
}

//--------------------------------------------------------------
// Test the base64::decode() function.
//--------------------------------------------------------------

TEST(Base64, Decode) {
    auto t = [] (char const * text) -> std::string {
        std::vector<uint8_t> v;
        if (!base64::decode(v, text)) {
            throw std::runtime_error("invalid base64");
        }
        return std::string(v.cbegin(), v.cend());
    };

    EXPECT_EQ(t(""),         ""       );
    EXPECT_EQ(t("Zg=="),     "f"      );
    EXPECT_EQ(t("Zm8="),     "fo"     );
    EXPECT_EQ(t("Zm9v"),     "foo"    );
    EXPECT_EQ(t("Zm9vYg=="), "foob"   );
    EXPECT_EQ(t("Zm9vYmE="), "fooba"  );
    EXPECT_EQ(t("Zm9vYmFy"), "foobar" );

    EXPECT_EQ(t("Z m9v YmFy"),   "foobar" );
    EXPECT_EQ(t("Zm9v\nYmFy\n"), "foobar" );
    EXPECT_EQ(t("Zm9\tvYmFy\r"), "foobar" );

    EXPECT_THROW(t("Z"),    std::runtime_error);
    EXPECT_THROW(t("Z==="), std::runtime_error);
    EXPECT_THROW(t("Z=gv"), std::runtime_error);
    EXPECT_THROW(t("Zg=v"), std::runtime_error);
}

//========================================================================
