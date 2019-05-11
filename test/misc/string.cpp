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
#include "misc/string.h"

//--------------------------------------------------------------
// Test the string::trim() function.
//--------------------------------------------------------------

TEST(String, trim) {
    auto f = [] (std::string const & s, string::mode m) {
        std::string ret(s);
        string::trim(ret, m);
        return ret;
    };

    EXPECT_EQ(f("", string::trim_left),            "");
    EXPECT_EQ(f("abc", string::trim_left),         "abc");
    EXPECT_EQ(f("   abc", string::trim_left),      "abc");
    EXPECT_EQ(f("abc   ", string::trim_left),      "abc   ");
    EXPECT_EQ(f("   abc   ", string::trim_left),   "abc   ");

    EXPECT_EQ(f("", string::trim_right),           "");
    EXPECT_EQ(f("abc", string::trim_right),        "abc");
    EXPECT_EQ(f("   abc", string::trim_right),     "   abc");
    EXPECT_EQ(f("abc   ", string::trim_right),     "abc");
    EXPECT_EQ(f("   abc   ", string::trim_right),  "   abc");

    EXPECT_EQ(f("", string::trim_both),            "");
    EXPECT_EQ(f("abc", string::trim_both),         "abc");
    EXPECT_EQ(f("   abc", string::trim_both),      "abc");
    EXPECT_EQ(f("abc   ", string::trim_both),      "abc");
    EXPECT_EQ(f("   abc   ", string::trim_both),   "abc");

    EXPECT_EQ(f(" \t\r\n\t ", string::trim_left),  "");
    EXPECT_EQ(f(" \t\r\n\t ", string::trim_right), "");
    EXPECT_EQ(f(" \t\r\n\t ", string::trim_both),  "");
}

//--------------------------------------------------------------
// Test the string::lowercase() function.
//--------------------------------------------------------------

TEST(String, lowercase) {
    auto f = [] (std::string const & s) {
        std::string ret(s);
        string::lowercase(ret);
        return ret;
    };
    EXPECT_EQ(f(""), "");
    EXPECT_EQ(f("a"), "a");
    EXPECT_EQ(f("X"), "x");
    EXPECT_EQ(f("YoU cAn'T aLwAyS gEt WhAt yOu wAnT"), "you can't always get what you want");
    EXPECT_EQ(f(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), " !\"#$%&'()*+,-./0123456789:;<=>?@abcdefghijklmnopqrstuvwxyz[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");
}

//--------------------------------------------------------------
// Test the string::compare_i() function.
//--------------------------------------------------------------

TEST(String, compare_i) {
    EXPECT_TRUE(string::compare_i("",     ""));
    EXPECT_TRUE(string::compare_i("b",    "b"));
    EXPECT_TRUE(string::compare_i("C",    "c"));
    EXPECT_TRUE(string::compare_i("c",    "C"));
    EXPECT_TRUE(string::compare_i("XY",   "xy"));
    EXPECT_TRUE(string::compare_i("xY",   "xy"));
    EXPECT_TRUE(string::compare_i("Xy",   "xy"));
    EXPECT_TRUE(string::compare_i("xy",   "xy"));

    EXPECT_FALSE(string::compare_i("",    "xyz"));
    EXPECT_FALSE(string::compare_i("xyz", ""));
    EXPECT_FALSE(string::compare_i("abc", "ab"));
    EXPECT_FALSE(string::compare_i("ab",  "abc"));
    EXPECT_FALSE(string::compare_i("abc", "xyz"));
}

//--------------------------------------------------------------
// Test the string::split() function.
//--------------------------------------------------------------

TEST(String, split) {
    auto f = [] (size_t off, std::string const & str) {
        std::string ret;
        string::split(str, ';', off, string::trim_both, [&ret] (std::string & r) { 
            if (!ret.empty()) {
                ret.push_back('|');
            }
            ret.append(r);
            return true;
        });
        return ret;
    };

    EXPECT_EQ(f(0,  ""),             "");
    EXPECT_EQ(f(0,  "abc"),          "abc");
    EXPECT_EQ(f(0,  "abc;"),         "abc");
    EXPECT_EQ(f(0,  "abc;def"),      "abc|def");
    EXPECT_EQ(f(0,  "abc;def;"),     "abc|def");
    EXPECT_EQ(f(0,  "abc;;def"),     "abc|def");
    EXPECT_EQ(f(0,  ";;;abc;;"),     "abc");
    EXPECT_EQ(f(0,  ";"),            "");
    EXPECT_EQ(f(0,  ";;"),           "");
    EXPECT_EQ(f(0,  "u;v;w;x;y;z"),  "u|v|w|x|y|z");
    EXPECT_EQ(f(1,  "u;v;w;x;y;z"),  "v|w|x|y|z");
    EXPECT_EQ(f(2,  "u;v;w;x;y;z"),  "v|w|x|y|z");
    EXPECT_EQ(f(3,  "u;v;w;x;y;z"),  "w|x|y|z");
    EXPECT_EQ(f(4,  "u;v;w;x;y;z"),  "w|x|y|z");
    EXPECT_EQ(f(5,  "u;v;w;x;y;z"),  "x|y|z");
    EXPECT_EQ(f(9,  "u;v;w;x;y;z"),  "z");
    EXPECT_EQ(f(10, "u;v;w;x;y;z"),  "z");
    EXPECT_EQ(f(11, "u;v;w;x;y;z"),  "");
}

//--------------------------------------------------------------
// Test the string::decodeURI() function.
//--------------------------------------------------------------

TEST(String, decodeURI) {
    EXPECT_EQ(string::decodeURI(""),     "");
    EXPECT_EQ(string::decodeURI("abc"),  "abc");
    EXPECT_EQ(string::decodeURI("a+bc"), "a bc");
    EXPECT_EQ(string::decodeURI("%23"),  "#");
    EXPECT_EQ(string::decodeURI("a%24"), "a$");
    EXPECT_EQ(string::decodeURI("%26 x"), "& x");

    EXPECT_THROW(string::decodeURI("%yz"), std::runtime_error);
    EXPECT_THROW(string::decodeURI("%0z"), std::runtime_error);
    EXPECT_THROW(string::decodeURI("%z1"), std::runtime_error);
    EXPECT_THROW(string::decodeURI("a%9"), std::runtime_error);
    EXPECT_THROW(string::decodeURI("a%"),  std::runtime_error);
}

//--------------------------------------------------------------
// Test the string::encodeHtml() function.
//--------------------------------------------------------------

TEST(String, encodeHtml) {
    EXPECT_EQ(string::encodeHtml(""),           "");
    EXPECT_EQ(string::encodeHtml("abc"),        "abc");
    EXPECT_EQ(string::encodeHtml(" <abc> "),    " &lt;abc&gt; ");
    EXPECT_EQ(string::encodeHtml("'\"&"),       "&apos;&quot;&amp;");
}

//--------------------------------------------------------------
// Test the string::to_long() function.
//--------------------------------------------------------------

TEST(String, to_long) {
    EXPECT_EQ(string::to_long("1", 10),             1);
    EXPECT_EQ(string::to_long("042", 10),          42);
    EXPECT_EQ(string::to_long("987654", 10),   987654);
    EXPECT_EQ(string::to_long("1ff81c", 16),  2095132);

    EXPECT_EQ(string::to_long(" 42", 10),          42);
    EXPECT_EQ(string::to_long("\t42", 10),         42);
    EXPECT_EQ(string::to_long("\r \n \r  42", 10), 42);

    EXPECT_EQ(string::to_long("42 ", 10),          42);
    EXPECT_EQ(string::to_long("42\t", 10),         42);
    EXPECT_EQ(string::to_long("42\r \n \r  ", 10), 42);

    EXPECT_EQ(string::to_long("", 10),             -1);
    EXPECT_EQ(string::to_long("xy", 10),           -1);
    EXPECT_EQ(string::to_long("42a", 10),          -1);
    EXPECT_EQ(string::to_long("z42", 10),          -1);

    EXPECT_EQ(string::to_long("-1", 10),           -1);
    EXPECT_EQ(string::to_long("-756", 10),         -1);
}

//========================================================================
