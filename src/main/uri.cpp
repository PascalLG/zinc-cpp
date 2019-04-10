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

#include <iostream>

#include "config.h"
#include "uri.h"
#include "resource_builtin.h"
#include "resource_error_page.h"
#include "resource_redirection.h"
#include "resource_directory.h"
#include "resource_static_file.h"
#include "resource_script.h"

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
// Resolve the URI and return the resource it points to. This
// is actually the heart of the HTTP server.
//--------------------------------------------------------------

std::shared_ptr<Resource> URI::resolve() const {
    try {

        // Canonicalize the URI by removing references to '.' and
        // '..'. When we hit an existing file, the URI is fully 
        // resolved and the remaining path is considered as part of
        // the PATH_INFO string.

        std::string uri, path_info;
        bool resolved = false;
        fs::type ft;

        string::split(path_, '/', 0, string::trim_none, [&] (std::string & s) {
            if (!resolved) {
                if (s == "..") {
                    size_t sep = uri.rfind('/');
                    if (sep == std::string::npos) {
                        throw 403;
                    }
                    uri.erase(sep);
                } else if (s != ".") {
                    uri.push_back('/');
                    uri.append(s);
                    ft = fs::makeFilepathFromURI(uri).getFileType();
                    if (ft == fs::file) {
                        resolved = true;
                    }
                }
            } else {
                path_info.push_back('/');
                path_info.append(s);
            }
            return true;
        });

        if (uri.empty()) {
            uri.push_back('/');
            ft = fs::makeFilepathFromURI(uri).getFileType();
        }

        // Check if the resource exists and is accessible, either
        // on disk or either has a built-in resource.

        if (ft == fs::errorNotFound) {
            std::shared_ptr<Resource> r = ResourceBuiltIn::resolve(uri);
            if (r) {
                return r;
            } else {
                throw 404;
            }
        } else if (ft == fs::errorPermission) {
            throw 403;
        } else if (ft == fs::errorOther) {
            throw 500;
        }

        // If the resource is a directory, try to find an index 
        // file. If there is not, return the directory content.

        if (ft == fs::directory) {
            for (std::string const & name: Configuration::getInstance().getDirectoryIndexes()) {
                std::string tmp = uri + '/' + name;
                if (fs::makeFilepathFromURI(tmp).getFileType() == fs::file) {
                    ft = fs::file;
                    uri = tmp;
                    break;
                }
            }

            bool listing = Configuration::getInstance().isListingEnabled();
            if (!listing && ft == fs::directory) {
                throw 403;  // no index file and directory listing disabled = permission denied
            }

            if (path_.back() != '/') {
                std::string location = getRequestURI(true);
                return std::make_shared<ResourceRedirection>(location, true);   // the URI is a directory and the final slash is missing = redirection
            }

            if (ft == fs::directory) {
                auto const & args = getArguments();
                return std::make_shared<ResourceDirectory>(uri, args);  // the URI is a directory without index file = show directory listing
            }
        }

        // The resource is a file. Depending on its extension, execute it as
        // a CGI script or return its content.

        fs::filepath filepath = fs::makeFilepathFromURI(uri);
        Configuration::CGI const * cgi = Configuration::getInstance().getInterpreter(filepath);
        if (cgi) {
            return std::make_shared<ResourceScript>(filepath, uri, path_info, *cgi);
        } else {
            std::ifstream fs(filepath.getStdString(), std::ifstream::in);
            if (!fs.good()) {
                throw 403;
            }
            return std::make_shared<ResourceStaticFile>(filepath, fs);
        }
    } catch (int err) {
        return std::make_shared<ResourceErrorPage>(err);
    }
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
