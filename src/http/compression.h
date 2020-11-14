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

#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <string>
#include <memory>

#include "stream.h"

class Mime;

//--------------------------------------------------------------
// HTTP compression handling.
//--------------------------------------------------------------

namespace compression {
    enum mode {
        none,
        zlib_gzip,
        zlib_deflate,
        brotli_generic,
        brotli_text,
        brotli_font,
    };

    class set {
    public:
        set() : set_(0)                                             {                                   }
        set(std::initializer_list<mode> il) : set_(0)               { for (mode m: il) { insert(m); }   }
        set(set const & other) : set_(other.set_)                   {                                   }

        set &   operator = (set const & other)                      { set_ = other.set_; return *this;  }

        void    insert(mode m)                                      { set_ |= 1u << m;                  }
        bool    contains(mode m) const                              { return (set_ & (1u << m)) != 0;   }
        bool    empty() const                                       { return set_ == 0;                 }

        friend bool  operator == (set const & lhs, set const & rhs) { return lhs.set_ == rhs.set_;      }
        friend bool  operator != (set const & lhs, set const & rhs) { return lhs.set_ != rhs.set_;      }

    private:
        uint_fast32_t set_;
    };
}

std::string                     getCompressionName(compression::mode mode);
compression::set                parseAcceptedEncodings(std::string const & str);
compression::mode               selectCompressionMode(compression::set accepted, Mime const & mimetype);
std::unique_ptr<OutputStream>   makeStreamTransformer(compression::mode mode, long length);

//--------------------------------------------------------------

#endif

//========================================================================
