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

#ifndef _WIN32
#include <cstdlib>
#include <unistd.h>
#endif

#include "blob.h"

//========================================================================
// blob
//
// Encapsulate a binary large object. The content is actually stored in a
// temporary file that is created the first time date are written and
// deleted when the last descriptor/handle is closed.
//
// The implementation MUST rely on plateform specific API and not on a
// higher level library (such as FILE* or std::fstream) because we need
// the raw file descriptor/handle to communicate with external processes.
//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

blob::blob()
  : fd_(INVALID_HANDLE_VALUE) {
}

//--------------------------------------------------------------
// Copy constructor.
//--------------------------------------------------------------

blob::blob(blob const & other)
  : fd_(INVALID_HANDLE_VALUE) {
#ifdef _WIN32
    DuplicateHandle(GetCurrentProcess(), other.fd_, GetCurrentProcess(), &fd_, 0, FALSE, DUPLICATE_SAME_ACCESS);
#else
    int f = dup(other.fd_);
    fd_ = f >= 0 ? f : INVALID_HANDLE_VALUE;
#endif
}

//--------------------------------------------------------------
// Destructor.
//--------------------------------------------------------------

blob::~blob() {
    if (IS_HANDLE_VALID(fd_)) {
        closefile(fd_);
    }
}

//--------------------------------------------------------------
// Assignment operator.
//--------------------------------------------------------------

blob & blob::operator = (blob other) {
    std::swap(fd_, other.fd_);
    return *this;
}

//--------------------------------------------------------------
// Write data to the file, creating and opening it if this is
// not already done.
//--------------------------------------------------------------

bool blob::write(void const * data, size_t length) {
#ifdef _WIN32
    if (!IS_HANDLE_VALID(fd_)) {
        WCHAR path[MAX_PATH], fname[MAX_PATH];
        GetTempPathW(MAX_PATH, path);
        GetTempFileNameW(path, L"tmp_", 0, fname);
        fd_ = CreateFileW(fname, GENERIC_READ | GENERIC_WRITE,  0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr);
        if (fd_ == INVALID_HANDLE_VALUE) {
            return false;
        }
    }
    DWORD written;
    return WriteFile(fd_, data, static_cast<DWORD>(length), &written, nullptr) && static_cast<size_t>(written) == length;
#else
    if (!IS_HANDLE_VALID(fd_)) {
        char fname[32] = "tmp_XXXXXX";
        fd_ = mkstemp(fname);
        if (fd_ < 0) {
            return false;
        }
        unlink(fname);
    }
    return ::write(fd_, data, length) == static_cast<ssize_t>(length);
#endif
}

//--------------------------------------------------------------
// Return the size of the file.
//--------------------------------------------------------------

size_t blob::getSize() const {
    size_t result;
#ifdef _WIN32
    if (IS_HANDLE_VALID(fd_)) {
        LARGE_INTEGER li;
        GetFileSizeEx(fd_, &li);
        result = static_cast<size_t>(li.QuadPart);
    } else {
        result = 0;
    }
#else
    if (IS_HANDLE_VALID(fd_)) {
        off_t previous = lseek(fd_, 0, SEEK_CUR);
        result = lseek(fd_, 0, SEEK_END);
        lseek(fd_, previous, SEEK_SET);
    } else {
        result = 0;
    }
#endif
    return result;
}

//--------------------------------------------------------------
// Read the whole file content in a vector.
//--------------------------------------------------------------

std::vector<uint8_t> blob::readAll() const {
    std::vector<uint8_t> content;
#ifdef _WIN32
    if (IS_HANDLE_VALID(fd_)) {
        LARGE_INTEGER li;
        GetFileSizeEx(fd_, &li);
        auto size = static_cast<size_t>(li.QuadPart);
        content.resize(size);
        SetFilePointer(fd_, 0, nullptr, FILE_BEGIN);
        DWORD read;
        ReadFile(fd_, content.data(), static_cast<DWORD>(size), &read, nullptr);
    }
#else
    if (IS_HANDLE_VALID(fd_)) {
        auto size = static_cast<size_t>(lseek(fd_, 0, SEEK_END));
        content.resize(size);
        lseek(fd_, 0, SEEK_SET);
        read(fd_, content.data(), size);
    }
#endif
    return content;
}

//========================================================================
