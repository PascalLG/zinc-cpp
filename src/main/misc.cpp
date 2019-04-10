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

#include "misc.h"

//========================================================================
// Helper string functions.
//========================================================================

std::string const string::empty;

//--------------------------------------------------------------
// Remove spaces at the beginning and/or the end of a string.
//--------------------------------------------------------------

void string::trim(std::string & s, string::mode m) {
    if (m & trim_left) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !isspace(ch); }));
    }
    if (m & trim_right) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [] (int ch) { return !isspace(ch); }).base(), s.end());
    }
}

//--------------------------------------------------------------
// Convert in place an ASCII string to lowercase.
//--------------------------------------------------------------

void string::lowercase(std::string & s) {
    std::transform(s.begin(), s.end(), s.begin(), tolower);
}

//--------------------------------------------------------------
// Test if two ASCII strings are equal, ignoring case.
//--------------------------------------------------------------

bool string::compare_i(std::string const & s1, std::string const & s2) {
    size_t n = s1.length();
    if (s2.length() != n) {
        return false;
    }
    for (size_t i = 0; i < n; i++) {
        int c1 = tolower(s1[i]);
        int c2 = tolower(s2[i]);
        if (c1 != c2) {
            return false;
        }
    }
    return true;
}

//--------------------------------------------------------------
// Split a string by a delimiter, calling the given callback for
// each extracted substring. If the callback returns false,
// substring enumeration stops immediately.
//--------------------------------------------------------------

void string::split(std::string const & str, char delimiter, size_t start, string::mode trim, std::function<bool(std::string &)> const & callback) {
    auto stub = [trim, &callback] (std::string s) {
        string::trim(s, trim);
        if (!s.empty()) {
            return callback(s);
        }
        return true;
    };
    for (bool ok = true; ok; ) {
        size_t sep = str.find(delimiter, start);
        if (sep == std::string::npos) {
            if (start < str.length()) {
                ok = stub(str.substr(start));
            }
            break;
        } else {
            if (sep > start) {
                ok = stub(str.substr(start, sep - start));
            }
            start = sep + 1;
        }
    }
}

//--------------------------------------------------------------
// Decode a URI. Plus signs (+) are replaced by spaces and
// percent-encoded characters are replaced by their actual
// value. Throw an exception if decoding fails.
//--------------------------------------------------------------

std::string string::decodeURI(std::string const & s) {

    auto decode = [] (int ch) {
        if (ch >= '0' && ch <= '9') {
            return ch - '0';
        } else if (ch >= 'A' && ch <= 'F') {
            return ch - 'A' + 10;
        } else if (ch >= 'a' && ch <= 'f') {
            return ch - 'a' + 10;
        } else {
            return -1;
        }
    };

    std::string ret;
    int state = 0, v1, v2;

    size_t len = s.length();
    for (size_t i = 0; i <= len; i++) {
        char ch = i < len ? s[i] : 0;
        switch (state) {
        case 0:
            if (ch == '+') {
                ret.push_back(' ');
            } else if (ch == '%') {
                state = 1;
            } else if (ch != '\0') {
                ret.push_back(ch);
            }
            break;
        case 1:
            if ((v1 = decode(ch)) < 0) {
                throw std::runtime_error("invalid URI encoding");
            }
            state = 2;
            break;
        case 2:
            if ((v2 = decode(ch)) < 0) {
                throw std::runtime_error("invalid URI encoding");
            }
            ret.push_back((v1 << 4) | v2);
            state = 0;
            break;
        }
    }
    return ret;
}

//--------------------------------------------------------------
// Encode HTML entities. Since all server-generated pages are UTF-8
// encoded, extended characters are simply represented by their
// unicode point; only mandatory entities need to processed.
//--------------------------------------------------------------

std::string string::encodeHtml(std::string const & s) {
    std::string result;
    result.reserve(s.size());

    for (char c : s) {
        switch (c) {
            case '&':   result.append("&amp;");     break;
            case '<':   result.append("&lt;");      break;
            case '>':   result.append("&gt;");      break;
            case '"':   result.append("&quot;");    break;
            case '\'':  result.append("&apos;");    break;
            default:    result.push_back(c);        break;
        }
    }

    return result;
}
    
//--------------------------------------------------------------
// Convert a string to a positive value. Returns a negative
// value if the conversion fails.
//--------------------------------------------------------------

long string::to_long(std::string const & str, int base) {
    long result = -1;
    if (!str.empty()) {
        char * err;
        long tmp = strtol(str.c_str(), &err, base);
        if (tmp >= 0) {
            while (isspace(*err)) {
                err++;
            }
            if (!*err) {
                result = tmp;
            }
        }
    }
    return result;
}

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
