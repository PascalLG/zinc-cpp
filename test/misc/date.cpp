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
#include "misc/date.h"

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
#ifdef _WIN32
	_putenv_s("TZ", "CET-01:00");
	_tzset();
#else
	setenv("TZ", "CET-01:00", 1);
	tzset();
#endif

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
