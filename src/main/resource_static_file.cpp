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

#include "../http/mimetype.h"
#include "zinc.h"
#include "resource_static_file.h"

//========================================================================
// ResourceStaticFile
//
// Resource consisting of a local static file. The URI resolver already
// does much of the job (opening the file, determining its MIME type, etc.)
// and this class is only responsible for emiting the HTTP headers and
// data.
//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

ResourceStaticFile::ResourceStaticFile(fs::filepath const & filename, std::ifstream & filestream)
    : Resource("static file " + filename.getStdString()),
      fileStream_(std::move(filestream)),
      mimeType_(getMimeType(filename, &fileStream_)),
      lastModified_(filename.getModificationDate()) {
}

//--------------------------------------------------------------
// Transmit the resource to the provided HttpResponse object.
//--------------------------------------------------------------

void ResourceStaticFile::transmit(HttpResponse & response, HttpRequest const & request) {
    date ifModifiedSince = date::from_http(request.getHeaderValue(HttpHeader::IfModifiedSince));
    if (lastModified_ > ifModifiedSince && request.getVerb().isOneOf(HttpVerb::Get | HttpVerb::Head)) {
        fileStream_.seekg(0, fileStream_.end);
        size_t size = static_cast<size_t>(fileStream_.tellg());

        response.emitHeader(HttpHeader::ContentType, mimeType_);
        response.emitHeader(HttpHeader::ContentLength, std::to_string(size));
        response.emitHeader(HttpHeader::LastModified, lastModified_.to_http());
        response.emitHeader(HttpHeader::Expires, response.getResponseDate().add(Zinc::getInstance().getConfiguration().getExpires()).to_http());
        response.emitEol();

        fileStream_.seekg(0, fileStream_.beg);
        while (size) {
            char buffer[1024];
            size_t count = std::min(size, sizeof(buffer));
            fileStream_.read(buffer, count);
            response.write(buffer, count);
            size -= count;
        }
    } else {
        response.setHttpStatus(304);
        response.emitHeader(HttpHeader::ContentLength, "0");    // to indicate the HttpResponse object that the body is empty
        response.emitEol();
    }

    response.flush();
}

//========================================================================
