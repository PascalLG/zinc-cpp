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
#include "zinc.h"
#include "resource_page_error.h"
#include "resource_error_page.h"

//========================================================================
// ResourceErrorPage
//
// Resource consisting of an error page. The constructor accepts an HTTP
// status and the object builds the appropriate HTTP headers and page
// content.
//========================================================================

//--------------------------------------------------------------
// Construct an error page from an HTTP status code.
//--------------------------------------------------------------

ResourceErrorPage::ResourceErrorPage(HttpStatus status)
    : Resource("error page " + std::to_string(status.getStatusCode())),
      status_(status) {
}

//--------------------------------------------------------------
// Transmit the resource to the provided HttpResponse object.
//--------------------------------------------------------------

void ResourceErrorPage::transmit(HttpResponse & response, HttpRequest const & request) {
    response.setHttpStatus(status_);
    response.emitHeader(HttpHeader::ContentType, "text/html; charset=utf-8");
    response.emitEol();

    response.emitPage(reinterpret_cast<char const *>(page_error_html), [&] (std::string const & field) {
        std::string ret;
        if (field == "server_version") {
            ret = string::encodeHtml(Zinc::getInstance().getVersionString());
        } else if (field == "server_name") {
            ret = string::encodeHtml(Zinc::getInstance().getConfiguration().getServerName());
        } else if (field == "server_addr") {
            ret = request.getLocalAddress().getAddressString();
        } else if (field == "server_port") {
            ret = request.getLocalAddress().getPortString();
        } else if (field == "errno") {
            ret = std::to_string(this->status_.getStatusCode());
        } else if (field == "errmsg") {
            ret = string::encodeHtml(this->status_.getStatusString());
        } else if (field == "description") {
            ret = string::encodeHtml(getErrorDescription());
        }
        return ret;
    });

    response.flush();
}

//--------------------------------------------------------------
// Build the detailed error description.
//--------------------------------------------------------------

std::string ResourceErrorPage::getErrorDescription() const {
    static ResourceErrorPage::Mapping map; // Thread-safe as of C++11
    auto got = map.mapByStatus_.find(status_.getStatusCode());
    return got != map.mapByStatus_.end() ? got->second : "No detailed description available.";
}

//--------------------------------------------------------------
// Constructor for the Mapping object.
//--------------------------------------------------------------

ResourceErrorPage::Mapping::Mapping()
  : mapByStatus_ {
        { 400,  "Your browser (or proxy) sent a request that this server could not understand."                                                         },
//        { 401,  "Wrong credentials. The server could not verify that you are authorized to access this URL. "                                           },
        { 403,  "You don't have permission to access this URL on this server."                                                                          },
        { 404,  "The requested URL was not found on this server."                                                                                       },
        { 405,  "This method is not allowed for the requested URL."                                                                                     },
//        { 410,  "The requested URL is no longer available on this server and there is no forwarding address."                                           },
//        { 411,  "This request requires a valid <code>Content-Length</code> header."                                                                     },
//        { 412,  "The precondition on the request for the URL failed positive evaluation."                                                               },
        { 413,  "This method does not allow the data transmitted, or the data volume exceeds the capacity limit."                                       },
        { 414,  "The length of the requested URL exceeds the capacity limit for this server. The request cannot be processed."                          },
        { 431,  "The length of the request headers exceeds the capacity limit for this server. The request cannot be processed."                          },
        { 500,  "The server encountered an internal error and was unable to complete your request."                                                     },
        { 501,  "The server does not support the action requested by the browser."                                                                      },
    } {
}

//========================================================================
