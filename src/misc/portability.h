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

#ifndef PORTABILITY_H
#define PORTABILITY_H

//--------------------------------------------------------------
// Definitions specific to Win32 SDK.
//--------------------------------------------------------------

#ifdef _WIN32

#include <Windows.h>
#include <string>

#undef min      // to avoid clash with std::min
#undef max      // to avoid clash with std::max

typedef HANDLE                  HANDLE_T;
#define IS_HANDLE_VALID(x)      (x != INVALID_HANDLE_VALUE)
#define closefile(x)            CloseHandle(x)

typedef SOCKET                  SOCKET_T;
#define IS_SOCKET_VALID(x)      (x != INVALID_SOCKET)

#define localtime_r(x, y)       localtime_s((y), (x))
#define gmtime_r(x, y)          gmtime_s((y), (x))
#define timegm(x)               _mkgmtime((x))
#define tzset_                  _tzset

time_t                          FileTimeToPOSIX(FILETIME const & ft);
std::string                     WideStringToUTF8(WCHAR const * pstr);
std::wstring                    UTF8ToWideString(std::string const & str);

#else

//--------------------------------------------------------------
// Definitions specific to POSIX.
//--------------------------------------------------------------

typedef int                     HANDLE_T;
#define INVALID_HANDLE_VALUE    (-1)
#define IS_HANDLE_VALID(x)      (x >= 0)
#define closefile(x)            ::close(x)

typedef int                     SOCKET_T;
#define INVALID_SOCKET          (-1)
#define IS_SOCKET_VALID(x)      (x >= 0)
#define closesocket(x)          ::close(x)

#define tzset_                  tzset

#endif

//--------------------------------------------------------------

#endif

//========================================================================
