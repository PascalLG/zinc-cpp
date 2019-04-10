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

#include "misc.h"
#include "http_verb.h"

//========================================================================
// HttpVerb
//
// Represent a HTTP verb. For the sake of efficiency, a verb is not
// represented as a string but as an enum value. Since most operations
// on verbs consist of determining whether a verb belongs to a given
// set, enum values are chosen to form a bitset and the | operator
// is overloaded to help built a set of verbs.
//
// Verbs are case sensitive. There is no support for custom verbs.
//========================================================================

//--------------------------------------------------------------
// Constructor from a name. 
//--------------------------------------------------------------

HttpVerb::HttpVerb(std::string const & name) {
    auto const & map = getMap().mapByName_;
    auto got = map.find(name);
    verb_ = (got != map.end()) ? got->second : Unknown;
}

//--------------------------------------------------------------
// Return the verb name. 
//--------------------------------------------------------------

std::string const & HttpVerb::getVerbName() const {
    auto const & map = getMap().mapByVerb_;
    auto got = map.find(verb_);
    return (got != map.end()) ? got->second : string::empty;
}

//--------------------------------------------------------------
// Constructor for the Mapping object. This inner class builds
// maps that allow for quick retrieval of a verb code from its
// name, or a verb name from its code.
//--------------------------------------------------------------

HttpVerb::Mapping::Mapping()
  : mapByVerb_ {
        { HttpVerb::GET,        "GET"       },
        { HttpVerb::HEAD,       "HEAD"      },
        { HttpVerb::POST,       "POST"      },
        { HttpVerb::PUT,        "PUT"       },
        { HttpVerb::DELETE,     "DELETE"    },
        { HttpVerb::CONNECT,    "CONNECT"   },
        { HttpVerb::OPTIONS,    "OPTIONS"   },
        { HttpVerb::TRACE,      "TRACE"     },
        { HttpVerb::PATCH,      "PATCH"     },
    },
    mapByName_ {} {

    for (auto & i: mapByVerb_) {
        mapByName_.emplace(i.second, i.first);
    }
}

//--------------------------------------------------------------
// Return a singleton Mapping object.
//--------------------------------------------------------------

HttpVerb::Mapping & HttpVerb::getMap() const {
    static HttpVerb::Mapping map; // Thread-safe as of C++11
    return map;
}

//========================================================================
