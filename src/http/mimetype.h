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

#ifndef MIMETYPE_H
#define MIMETYPE_H

#include <string>
#include <istream>

#include "../misc/filesys.h"
#include "compression.h"

//--------------------------------------------------------------
// MIME type.
//--------------------------------------------------------------

class Mime {
public:
    explicit Mime(std::string mimetype);
    Mime(fs::filepath const & filename, std::istream * content);
    Mime(Mime const & other) = default;
    Mime & operator = (Mime const & other) = default;

    std::string         toString() const    { return mimetype_; }
    compression::mode   getFavoriteCompressionMode() const;

private:
    std::string     mimetype_;
};

//--------------------------------------------------------------
// Text functions. (Used internally but made public for unit
// testing purposes.)
//--------------------------------------------------------------

std::string guessEncoding(std::istream & is);
bool        isUTF8(char const * text, size_t length, bool * ascii);
bool        isUTF16(char const * text, size_t length, bool bigendian);
bool        isValidUnicodeChar(uint32_t ch);

//--------------------------------------------------------------

#endif

//========================================================================
