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

#include "portability.h"

//--------------------------------------------------------------
// Convert a Windows FILETIME structure to a POSIX timestamp.
//--------------------------------------------------------------

#ifdef _WIN32

time_t FileTimeToPOSIX(FILETIME & ft) {
    LARGE_INTEGER date, adjust;
    date.HighPart = ft.dwHighDateTime;
    date.LowPart = ft.dwLowDateTime;
    date.QuadPart -= 11644473600000ll * 10000ll;
    return static_cast<time_t>(date.QuadPart / 10000000ll);
}

#endif

//--------------------------------------------------------------
// Convert a WCHAR string to a std::string.
//--------------------------------------------------------------

#ifdef _WIN32

std::string WideStringToUTF8(WCHAR * pstr) {

    return std::string();
}

#endif

//--------------------------------------------------------------

#ifdef _WIN32

std::u16string UTF8ToWideString(std::string const & str) {

    return std::u16string();
}

#endif

//========================================================================
