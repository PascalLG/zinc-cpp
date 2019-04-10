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

#ifndef __RESOURCE_ERROR_PAGE_H__
#define __RESOURCE_ERROR_PAGE_H__

#include "version.h"
#include "resource.h"
#include "http_status.h"

//--------------------------------------------------------------
// Resource consisting of an error page.
//--------------------------------------------------------------

class ResourceErrorPage : public Resource {
public:
	ResourceErrorPage(HttpStatus status);

	void transmit(HttpResponse & response, HttpRequest const & request) override;

private:
	HttpStatus status_;

    class Mapping {
    public:
        Mapping();
        std::unordered_map<int, std::string> mapByStatus_;
    };

public_for_testing:
    std::string getErrorDescription() const;
};

//--------------------------------------------------------------

#endif

//========================================================================
