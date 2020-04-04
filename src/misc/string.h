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

#ifndef STRING_H
#define STRING_H

#include <string>
#include <functional>

namespace string {

//--------------------------------------------------------------
// Helper string functions.
//--------------------------------------------------------------

extern std::string const empty;

enum mode {
    trim_none    = 0x00,
    trim_left    = 0x01,
    trim_right   = 0x02,
    trim_both    = (trim_left | trim_right),
};

void        trim(std::string & s, mode m);
void        lowercase(std::string & s);
bool        compare_i(std::string const & s1, std::string const & s2);
void        split(std::string const & str, char delimiter, size_t start, mode trim, std::function<bool(std::string &)> callback);
std::string decodeURI(std::string const & s);
std::string encodeHtml(std::string const & s);
long        to_long(std::string const & str, int base);

//--------------------------------------------------------------

}

#endif

//========================================================================
