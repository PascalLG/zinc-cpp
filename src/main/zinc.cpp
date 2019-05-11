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

#include <iostream>    // TODO: à supprimer
#include "version.h"
#include "resource_builtin.h"
#include "resource_directory.h"
#include "resource_error_page.h"
#include "resource_redirection.h"
#include "resource_script.h"
#include "resource_static_file.h"
#include "zinc.h"

//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

Zinc::Zinc()
  : configuration_() {
}

//--------------------------------------------------------------
// Resolve a URI and return the resource it points to.
//--------------------------------------------------------------

std::shared_ptr<Resource> Zinc::resolve(URI const & uri) {
    try {

        // Canonicalize the URI by removing references to '.' and
        // '..'. When we hit an existing file, the URI is fully 
        // resolved and the remaining path is considered as part of
        // the PATH_INFO string.

        std::string uripath, path_info;
        bool resolved = false;
        fs::type ft;

        string::split(uri.getPath(), '/', 0, string::trim_none, [&] (std::string & s) {
            if (!resolved) {
                if (s == "..") {
                    size_t sep = uripath.rfind('/');
                    if (sep == std::string::npos) {
                        throw 403;
                    }
                    uripath.erase(sep);
                } else if (s != ".") {
                    uripath.push_back('/');
                    uripath.append(s);
                    ft = fs::makeFilepathFromURI(uripath).getFileType();
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

        if (uripath.empty()) {
            ft = fs::makeFilepathFromURI(uripath).getFileType();
        }

        // Check if the resource exists and is accessible, either
        // on disk or either has a built-in resource.

        if (ft == fs::errorNotFound) {
            std::shared_ptr<Resource> r = ResourceBuiltIn::resolve(uripath);
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
            for (std::string const & name: configuration_.getDirectoryIndexes()) {
                std::string tmp = uripath + '/' + name;
                if (fs::makeFilepathFromURI(tmp).getFileType() == fs::file) {
                    ft = fs::file;
                    uripath = tmp;
                    break;
                }
            }

            bool listing = configuration_.isListingEnabled();
            if (!listing && ft == fs::directory) {
                throw 403;  // no index file and directory listing disabled = permission denied
            }

            if (uri.getPath().back() != '/') {
                std::string location = uri.getRequestURI(true);
                return std::make_shared<ResourceRedirection>(location, true);   // the URI is a directory and the final slash is missing = redirection
            }

            if (ft == fs::directory) {
                if (uripath.empty()) { 
                    uripath.push_back('/');
                }
                auto const & args = uri.getArguments();
                return std::make_shared<ResourceDirectory>(uripath, args);  // the URI is a directory without index file = show directory listing
            }
        }

        // The resource is a file. Depending on its extension, execute it as
        // a CGI script or return its content.

        fs::filepath filepath = fs::makeFilepathFromURI(uripath);
        Configuration::CGI const * cgi = configuration_.getInterpreter(filepath);
        if (cgi) {
            return std::make_shared<ResourceScript>(filepath, uripath, path_info, *cgi);
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
// Return a resource corresponding to an error page.
//--------------------------------------------------------------

std::shared_ptr<Resource> Zinc::makeErrorPage(HttpStatus status) {
    return std::make_shared<ResourceErrorPage>(status);
}

//--------------------------------------------------------------
// Return a version string containing the server name, major and
// minor version, and the date the server was built.
//--------------------------------------------------------------

std::string Zinc::getVersionString() {
    char sz[256];
    sprintf(sz, "Zinc/%d.%02d (" ZINC_PLATEFORM_NAME ")", ZINC_VERSION_MAJOR, ZINC_VERSION_MINOR);
    return std::string(sz);
}

//--------------------------------------------------------------
// Return the singleton configuration object.
//--------------------------------------------------------------

Zinc & Zinc::getInstance() {
    static Zinc instance;  // Thread-safe as of C++11
    return instance;
}

//========================================================================
