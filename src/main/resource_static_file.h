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

#ifndef __RESOURCE_STATIC_FILE_H__
#define __RESOURCE_STATIC_FILE_H__

#include <fstream>
#include "../misc/filesys.h"
#include "../http/resource.h"

//--------------------------------------------------------------
// Resource consisting of a static file.
//--------------------------------------------------------------

class ResourceStaticFile : public Resource {
public:
    ResourceStaticFile(fs::filepath const & filename, std::ifstream & filestream);

    void transmit(HttpResponse & response, HttpRequest const & request) override;

private:
    std::ifstream   fileStream_;
    std::string     mimeType_;
    date            lastModified_;
};

//--------------------------------------------------------------

#endif

//========================================================================
