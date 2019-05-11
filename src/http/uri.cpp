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

#include "../misc/string.h"
#include "uri.h"

//========================================================================
// URI
//
// Represent a URI. Provide methods to parse a URI string and extract the
// path and query parts, and to resolve and retrieve the resource
// pointed to by a URI.
//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

URI::URI()
    : path_(),
      query_(),
      arguments_() {
}

//--------------------------------------------------------------
// Parse and validate a URI.
//--------------------------------------------------------------

bool URI::parse(std::string const & uri) {
    clear();
    try {
        size_t qpos = uri.find('?');
        if (qpos == std::string::npos) {
            path_ = string::decodeURI(uri);
        } else {
            path_ = string::decodeURI(uri.substr(0, qpos++));
            if (uri.find('?', qpos) != std::string::npos) {
                throw std::runtime_error("multiple query parts");
            }
            query_ = uri.substr(qpos);
            string::split(query_, ';', 0, string::trim_none, [this] (std::string & arg) {
                size_t sep = arg.find('=');
                if (sep == std::string::npos) {
                    this->arguments_.emplace(string::decodeURI(arg), string::empty);
                } else {
                    this->arguments_.emplace(string::decodeURI(arg.substr(0, sep)), string::decodeURI(arg.substr(sep + 1)));
                }
                return true;
            });
        }
    } catch (std::runtime_error const &) {
        clear();
        return false;
    }
    return true;
}

//--------------------------------------------------------------
// Clear the object content.
//--------------------------------------------------------------

void URI::clear() {
    path_.clear();
    query_.clear();
    arguments_.clear();
}

//--------------------------------------------------------------
// Return the request URI by joining the path and the query
// parts. If we known that the path refers to a directory,
// also insert a slash at the end of the path.
//--------------------------------------------------------------

std::string URI::getRequestURI(bool directory) const {
    std::string req = path_;
    if (directory && req.back() !='/') {
        req.push_back('/');
    }
    if (!query_.empty()) {
        req.push_back('?');
        req.append(query_);
    }
    return req;
}

//========================================================================
