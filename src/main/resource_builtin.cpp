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

#include "resource_image_back.h"
#include "resource_image_folder.h"
#include "resource_image_document.h"
#include "resource_image_arrow_up.h"
#include "resource_image_arrow_down.h"
#include "resource_style.h"
#include "resource_titilliumweb.h"

#include "../http/mimetype.h"
#include "version.h"
#include "resource_builtin.h"

//========================================================================
// ResourceBuiltIn
//
// Resource consisting of a built-in image. These images are embedded in
// the application (see the image_*.h include files above) and are used
// by server generated pages such as error pages and directory listings.
//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

ResourceBuiltIn::ResourceBuiltIn(fs::filepath const & resource, void const * data, unsigned int length)
    : Resource("built-in " + resource.getStdString()),
      resource_(resource),
      data_(data),
      length_(length) {
}

//--------------------------------------------------------------
// Check if a given URI matches one of the predefined built-in
// resources. We use a map that is built the first time this
// method is called.
//--------------------------------------------------------------

std::shared_ptr<Resource> ResourceBuiltIn::resolve(std::string const & uri) {
    static ResourceBuiltIn::Mapping map; // Thread-safe as of C++11
    std::shared_ptr<Resource> ret;

    auto got = map.mapByName_.find(uri);
    if (got != map.mapByName_.end()) {
        ret = got->second;
    }

    return ret;
}

//--------------------------------------------------------------
// Transmit the resource to the provided HttpResponse object.
//--------------------------------------------------------------

void ResourceBuiltIn::transmit(HttpResponse & response, HttpRequest const & request) {
    static date lastModified = date(ZINC_BUILD_TIMESTAMP);

    date ifModifiedSince = date::from_http(request.getHeaderValue(HttpHeader::IfModifiedSince));
    if (lastModified > ifModifiedSince && request.getVerb().isOneOf(HttpVerb::Get | HttpVerb::Head)) {
        response.emitHeader(HttpHeader::ContentType, getMimeType(resource_, nullptr));
        response.emitHeader(HttpHeader::ContentLength, std::to_string(length_));
        response.emitHeader(HttpHeader::LastModified, lastModified.to_http());
        response.emitHeader(HttpHeader::Expires, response.getResponseDate().add(31557600).to_http()); // about 1 year
        response.emitEol();
        response.write(data_, length_);
    } else {
        response.setHttpStatus(304);
        response.emitHeader(HttpHeader::ContentLength, "0");    // to indicate the HttpResponse object that the body is empty
        response.emitEol();
    }

    response.flush();
}

//--------------------------------------------------------------
// Constructor for the Mapping object. Internal URIs for the
// build-in images start with /__zinc__/ to minimize the chances
// of a collision with resources on the actual file system.
//--------------------------------------------------------------

ResourceBuiltIn::Mapping::Mapping()
  : mapByName_ {
        { "/__zinc__/folder.png",               std::make_shared<ResourceBuiltIn>("folder.png",                 image_folder_png,           image_folder_png_length         ) },
        { "/__zinc__/back.png",                 std::make_shared<ResourceBuiltIn>("back.png",                   image_back_png,             image_back_png_length           ) },
        { "/__zinc__/document.png",             std::make_shared<ResourceBuiltIn>("document.png",               image_document_png,         image_document_png_length       ) },
        { "/__zinc__/arrow_up.png",             std::make_shared<ResourceBuiltIn>("arrow_up.png",               image_arrow_up_png,         image_arrow_up_png_length       ) },
        { "/__zinc__/arrow_down.png",           std::make_shared<ResourceBuiltIn>("arrow_down.png",             image_arrow_down_png,       image_arrow_down_png_length     ) },
        { "/__zinc__/style.css",                std::make_shared<ResourceBuiltIn>("style.css",                  style_css,                  style_css_length                ) },
        { "/__zinc__/TitilliumWeb-Regular.ttf", std::make_shared<ResourceBuiltIn>("TitilliumWeb-Regular.ttf",   titilliumweb_regular_ttf,   titilliumweb_regular_ttf_length ) },
    } {
}

//========================================================================
