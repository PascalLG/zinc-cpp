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
#include "misc.h"

//--------------------------------------------------------------
// Test the string::trim() function.
//--------------------------------------------------------------

TEST(Misc, trim) {
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

TEST(Misc, lowercase) {
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

TEST(Misc, compare_i) {
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

TEST(Misc, split) {
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

TEST(Misc, decodeURI) {
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

TEST(Misc, encodeHtml) {
    EXPECT_EQ(string::encodeHtml(""),           "");
    EXPECT_EQ(string::encodeHtml("abc"),        "abc");
    EXPECT_EQ(string::encodeHtml(" <abc> "),    " &lt;abc&gt; ");
    EXPECT_EQ(string::encodeHtml("'\"&"),       "&apos;&quot;&amp;");
}

//--------------------------------------------------------------
// Test the string::to_long() function.
//--------------------------------------------------------------

TEST(Misc, to_long) {
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

//--------------------------------------------------------------
// Test the date::valid() function.
//--------------------------------------------------------------

TEST(Date, Valid) {
    EXPECT_FALSE(date().valid());
    EXPECT_TRUE(date(0).valid());
    EXPECT_TRUE(date(1549007912).valid());
}

//--------------------------------------------------------------
// Test the date formating/parsing functions.
//--------------------------------------------------------------

TEST(Date, Format) {
    setenv("TZ", "CET-01:00", 1);
    tzset();

    EXPECT_EQ(date(0).format("%Y-%m-%d %H:%M:%S", date::gmt),               "1970-01-01 00:00:00");
    EXPECT_EQ(date(1).format("%Y-%m-%d %H:%M:%S", date::gmt),               "1970-01-01 00:00:01");
    EXPECT_EQ(date(3599).format("%Y-%m-%d %H:%M:%S", date::gmt),            "1970-01-01 00:59:59");
    EXPECT_EQ(date(3600).format("%Y-%m-%d %H:%M:%S", date::gmt),            "1970-01-01 01:00:00");
    EXPECT_EQ(date(86399).format("%Y-%m-%d %H:%M:%S", date::gmt),           "1970-01-01 23:59:59");
    EXPECT_EQ(date(86400).format("%Y-%m-%d %H:%M:%S", date::gmt),           "1970-01-02 00:00:00");
    EXPECT_EQ(date(1549007912).format("%Y-%m-%d %H:%M:%S", date::gmt),      "2019-02-01 07:58:32");

    EXPECT_EQ(date(0).format("%Y-%m-%d %H:%M:%S", date::local),             "1970-01-01 01:00:00");
    EXPECT_EQ(date(1549007912).format("%Y-%m-%d %H:%M:%S", date::local),    "2019-02-01 08:58:32");

    EXPECT_EQ(date(1).to_http(),                                            "Thu, 01 Jan 1970 00:00:01 GMT");
    EXPECT_EQ(date(1549007912).to_http(),                                   "Fri, 01 Feb 2019 07:58:32 GMT");

    EXPECT_EQ(date::from_http("Thu, 01 Jan 1970 00:00:01 GMT"),             date(1));
    EXPECT_EQ(date::from_http("Fri, 01 Feb 2019 07:58:32 GMT"),             date(1549007912));

    EXPECT_FALSE(date::from_http("abcd").valid());
    EXPECT_FALSE(date::from_http("01 Jan 1970 00:00:01 GMT").valid());
    EXPECT_FALSE(date::from_http("Thu, 01 Jan 1970 00:00:01").valid());
    EXPECT_FALSE(date::from_http("").valid());
}

//--------------------------------------------------------------
// Test the date comparison functions.
//--------------------------------------------------------------

TEST(Date, Compare) {
    EXPECT_FALSE(date()     < date());
    EXPECT_FALSE(date(1000) < date());
    EXPECT_FALSE(date(2000) < date());
    EXPECT_TRUE (date()     < date(1000));
    EXPECT_FALSE(date(1000) < date(1000));
    EXPECT_FALSE(date(2000) < date(1000));
    EXPECT_TRUE (date()     < date(2000));
    EXPECT_TRUE (date(1000) < date(2000));
    EXPECT_FALSE(date(2000) < date(2000));

    EXPECT_TRUE (date()     <= date());
    EXPECT_FALSE(date(1000) <= date());
    EXPECT_FALSE(date(2000) <= date());
    EXPECT_TRUE (date()     <= date(1000));
    EXPECT_TRUE (date(1000) <= date(1000));
    EXPECT_FALSE(date(2000) <= date(1000));
    EXPECT_TRUE (date()     <= date(2000));
    EXPECT_TRUE (date(1000) <= date(2000));
    EXPECT_TRUE (date(2000) <= date(2000));

    EXPECT_FALSE(date()     > date());
    EXPECT_TRUE (date(1000) > date());
    EXPECT_TRUE (date(2000) > date());
    EXPECT_FALSE(date()     > date(1000));
    EXPECT_FALSE(date(1000) > date(1000));
    EXPECT_TRUE (date(2000) > date(1000));
    EXPECT_FALSE(date()     > date(2000));
    EXPECT_FALSE(date(1000) > date(2000));
    EXPECT_FALSE(date(2000) > date(2000));

    EXPECT_TRUE (date()     >= date());
    EXPECT_TRUE (date(1000) >= date());
    EXPECT_TRUE (date(2000) >= date());
    EXPECT_FALSE(date()     >= date(1000));
    EXPECT_TRUE (date(1000) >= date(1000));
    EXPECT_TRUE (date(2000) >= date(1000));
    EXPECT_FALSE(date()     >= date(2000));
    EXPECT_FALSE(date(1000) >= date(2000));
    EXPECT_TRUE (date(2000) >= date(2000));

    EXPECT_TRUE (date()     == date());
    EXPECT_FALSE(date(1000) == date());
    EXPECT_FALSE(date(2000) == date());
    EXPECT_FALSE(date()     == date(1000));
    EXPECT_TRUE (date(1000) == date(1000));
    EXPECT_FALSE(date(2000) == date(1000));
    EXPECT_FALSE(date()     == date(2000));
    EXPECT_FALSE(date(1000) == date(2000));
    EXPECT_TRUE (date(2000) == date(2000));

    EXPECT_FALSE(date()     != date());
    EXPECT_TRUE (date(1000) != date());
    EXPECT_TRUE (date(2000) != date());
    EXPECT_TRUE (date()     != date(1000));
    EXPECT_FALSE(date(1000) != date(1000));
    EXPECT_TRUE (date(2000) != date(1000));
    EXPECT_TRUE (date()     != date(2000));
    EXPECT_TRUE (date(1000) != date(2000));
    EXPECT_FALSE(date(2000) != date(2000));
}

//--------------------------------------------------------------
// Test the date::add() function.
//--------------------------------------------------------------

TEST(Date, Add) {
    EXPECT_EQ(date(534122717).add(-86400).format("%Y-%m-%d %H:%M:%S", date::gmt), "1986-12-03 23:25:17");
    EXPECT_EQ(date(534122717).add( -3600).format("%Y-%m-%d %H:%M:%S", date::gmt), "1986-12-04 22:25:17");
    EXPECT_EQ(date(534122717).add(   -60).format("%Y-%m-%d %H:%M:%S", date::gmt), "1986-12-04 23:24:17");
    EXPECT_EQ(date(534122717).add(    -1).format("%Y-%m-%d %H:%M:%S", date::gmt), "1986-12-04 23:25:16");
    EXPECT_EQ(date(534122717).add(     0).format("%Y-%m-%d %H:%M:%S", date::gmt), "1986-12-04 23:25:17");
    EXPECT_EQ(date(534122717).add(    +1).format("%Y-%m-%d %H:%M:%S", date::gmt), "1986-12-04 23:25:18");
    EXPECT_EQ(date(534122717).add(   +60).format("%Y-%m-%d %H:%M:%S", date::gmt), "1986-12-04 23:26:17");
    EXPECT_EQ(date(534122717).add( +3600).format("%Y-%m-%d %H:%M:%S", date::gmt), "1986-12-05 00:25:17");
    EXPECT_EQ(date(534122717).add(+86400).format("%Y-%m-%d %H:%M:%S", date::gmt), "1986-12-05 23:25:17");
}

//========================================================================
