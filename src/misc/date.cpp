//========================================================================
// Zinc - Web Server
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

#include <cassert>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include "portability.h"
#include "date.h"

//========================================================================
// Date
//
// Encapsulate a time_t into an object, for improved expressiveness,
// portability and consistency. Also implements a "not-a-date" special
// value, to deal with missing value and/or errors.
//
// Note: the implementation is somewhat convoluted because we have to 
// account for systems where time_t is unsigned.
//========================================================================

//--------------------------------------------------------------
// Default constructor. Construct an invalid date, i.e. a date
// set to the maximum representable value for the time_t type.
//--------------------------------------------------------------

date::date()
  : timestamp_(std::numeric_limits<time_t>::max()) {
}

//--------------------------------------------------------------
// Construct a date from a UNIX timestamp.
//--------------------------------------------------------------

date::date(time_t timestamp)
  : timestamp_(timestamp) {
    assert(valid());
}

//--------------------------------------------------------------
// Indicates whether a date is valid.
//--------------------------------------------------------------

bool date::valid() const {
    return timestamp_ != std::numeric_limits<time_t>::max();
}

//--------------------------------------------------------------
// Add a given number of seconds to a date. (For the sake of
// efficiency, we don't check for overflows: given our use cases,
// they are very unlikely to happen.)
//--------------------------------------------------------------

date date::add(int interval) const {
    assert(valid());
    return date(timestamp_ + interval);
}

//--------------------------------------------------------------
// Compare two dates. An invalid date is less than all other
// dates. Two invalid dates are equal.
//--------------------------------------------------------------

int date::compare(date const & rhs) const {
    int64_t l = valid() ? timestamp_ : std::numeric_limits<int64_t>::min();
    int64_t r = rhs.valid() ? rhs.timestamp_ : std::numeric_limits<int64_t>::min();
    return (l < r) ? -1 : ((l > r) ? +1 : 0);
}

//--------------------------------------------------------------
// Print a date/time to a string, according to the specified
// format and timezone.
//--------------------------------------------------------------

std::string date::format(char const * format, timezone zone) const {
    struct tm tm;
    switch (zone) {
		case local: localtime_r(&timestamp_, &tm);    break;
		default:    gmtime_r(&timestamp_, &tm);       break;
	}

    std::ostringstream oss;
    oss << std::put_time(&tm, format);

    return oss.str();
}

//--------------------------------------------------------------
// Print a date/time to a string, according to the format expected
// in HTTP headers.
//--------------------------------------------------------------

std::string date::to_http() const {
    return format("%a, %d %b %Y %H:%M:%S GMT", gmt);
}

//--------------------------------------------------------------
// Return the current date/time.
//--------------------------------------------------------------

date date::now() {
    return date(time(nullptr));
}

//--------------------------------------------------------------
// Parse a string containing a date/time in the format expected
// in HTTP headers. Return an invalid date if parsing fails.
//--------------------------------------------------------------

date date::from_http(std::string const & time) {
    size_t len = time.length();
    if (len >= 28 && time.compare(len - 4, 4, " GMT") == 0) {
        struct tm tm;
        std::istringstream iss(time);
        iss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
        if (!iss.fail()) {
            return date(timegm(&tm));
        }
    }
    return date();
}

//========================================================================
