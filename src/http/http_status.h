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

#ifndef HTTP_STATUS_H
#define HTTP_STATUS_H

#include <string>
#include <unordered_map>

//--------------------------------------------------------------
// HTTP status code.
//--------------------------------------------------------------

class HttpStatus {
public:
    HttpStatus() : status_(0)                                                       {                                           }
    HttpStatus(int status) : status_(status)                                        {                                           }
    HttpStatus(HttpStatus const & other) : status_(other.status_)                   {                                           }

    HttpStatus &        operator = (HttpStatus const & other)                        { status_ = other.status_; return *this;   }

    friend bool         operator == (HttpStatus const & lhs, HttpStatus const & rhs) { return lhs.status_ == rhs.status_;       }
    friend bool         operator != (HttpStatus const & lhs, HttpStatus const & rhs) { return lhs.status_ != rhs.status_;       }

    int                 getStatusCode() const                                        { return status_;                          }
    std::string const & getStatusString() const;

private:
    int status_;

    class Mapping {
    public:
        Mapping();
        std::unordered_map<int, std::string> mapByStatus_;
    };

    Mapping & getMap() const;
};

//--------------------------------------------------------------

#endif

//========================================================================
