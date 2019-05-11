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

#ifndef __HTTP_VERB_H__
#define __HTTP_VERB_H__

#include <string>
#include <unordered_map>

//--------------------------------------------------------------
// HTTP verb.
//--------------------------------------------------------------

class HttpVerb {
public:
    enum Verb {
        Unknown     = 0x0000,
        Get         = 0x0001,
        Head        = 0x0002,
        Post        = 0x0004,
        Put         = 0x0008,
        Delete      = 0x0010,
        Connect     = 0x0020,
        Options     = 0x0040,
        Trace       = 0x0080,
        Patch       = 0x0100,
    };

    friend Verb operator | (Verb lhs, Verb rhs)                                         { return (Verb) ((unsigned) lhs | (unsigned) rhs);          }

    HttpVerb() : verb_(Unknown)                                                         {                                                           }
    HttpVerb(Verb verb) : verb_(verb)                                                   {                                                           }
    HttpVerb(std::string const & verb);
    HttpVerb(HttpVerb const & other) : verb_(other.verb_)                               {                                                           }

    HttpVerb &              operator = (HttpVerb const & other)                         { verb_ = other.verb_; return *this;                        }
    friend bool             operator == (HttpVerb const & lhs, HttpVerb const & rhs)    { return lhs.verb_ == rhs.verb_ && lhs.verb_ != Unknown;    }
    friend std::ostream &   operator << (std::ostream & os, HttpVerb const & rhs)       { return os << rhs.getVerbName();                           }
    bool                    isValid() const                                             { return verb_ != Unknown;                                  }
    bool                    isOneOf(Verb set) const                                     { return (verb_ & set) != 0;                                }

    std::string const &     getVerbName() const;

private:
    Verb verb_;

    class Mapping {
    public:
        Mapping();
        std::unordered_map<Verb, std::string, std::hash<int>> mapByVerb_;
        std::unordered_map<std::string, Verb>                 mapByName_;
    };

    Mapping & getMap() const;
};

//--------------------------------------------------------------

#endif

//========================================================================
