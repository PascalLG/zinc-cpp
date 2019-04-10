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

#include "resource_page_redirection.h"
#include "misc.h"
#include "resource_redirection.h"

//========================================================================
// ResourceRedirection
//
// Resource consisting of a redirection page. The constructor accepts
// a new path and a flag indicating whether the redirection is temporary
// or permanent, and the object builds the appropriate HTTP headers and
// page content.
//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

ResourceRedirection::ResourceRedirection(std::string const & location, bool permanent)
    : Resource("redirection to " + location),
      location_(location),
      permanent_(permanent) {
}

//--------------------------------------------------------------
// Transmit the resource to the provided HttpResponse object.
//--------------------------------------------------------------

void ResourceRedirection::transmit(HttpResponse & response, HttpRequest const & request) {
    HttpStatus status = getRedirectionStatus(request);
    std::string loc = getAbsoluteLocation(request);

    response.setHttpStatus(status);
    response.emitHeader(HttpHeader::ContentType, "text/html; charset=UTF-8");
    response.emitHeader(HttpHeader::Location, loc);
    response.emitEol();

    response.emitPage(reinterpret_cast<char const *>(page_redirection_html), [&] (std::string const & field) {
        std::string ret;
        if (!getFieldValue(ret, field, request)) {
            if (field == "status") {
                ret = std::to_string(status.getStatusCode());
            } else if (field == "description") {
                ret = string::encodeHtml(status.getStatusString());
            } else if (field == "location") {
                ret = string::encodeHtml(loc);
            }
        }
        return ret;
    });

    response.flush();
}

//--------------------------------------------------------------
// Build the absolute URI from the new location and from the
// client request.
//--------------------------------------------------------------

std::string ResourceRedirection::getAbsoluteLocation(HttpRequest const & request) {
    std::string loc = request.isSecureHTTP() ? "https" : "http";
    loc.append("://");
    loc.append(request.getHeaderValue(HttpHeader::Host));
    if (loc.back() != '/' && location_.front() != '/') {
        loc.push_back('/');
    } else if (loc.back() == '/' && location_.front() == '/') {
        loc.pop_back();
    }
    loc.append(location_);
    return loc;
}

//--------------------------------------------------------------
// Determine the proper HTTP status from the protocol version.
//--------------------------------------------------------------

HttpStatus ResourceRedirection::getRedirectionStatus(HttpRequest const & request) {
    if (request.getHttpVersion() >= HTTP_VERSION_1_1) {
        return permanent_ ? 308 : 307;
    } else {
        return permanent_ ? 301 : 302;
    }
}

//========================================================================
