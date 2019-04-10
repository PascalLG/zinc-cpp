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

#include "logger.h"
#include "config.h"
#include "resource.h"

//========================================================================
// Resource
//
// Abstract class that represents a resource to be sent to a client. Derived
// classes represent actual resources (files, CGI scripts, built-in pages,
// etc.) and must implement the transmit() method.
//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

Resource::Resource(std::string const & description)
	: description_(description) {

	LOG_TRACE("Init resource: " << description_);
}

//--------------------------------------------------------------
// Destructor.
//--------------------------------------------------------------

Resource::~Resource() {
	LOG_TRACE("Destroy resource: " << description_);
}

//--------------------------------------------------------------
// Build the value of some common fields on HTML page templates.
//--------------------------------------------------------------

bool Resource::getFieldValue(std::string & result, std::string const & field, HttpRequest const & request) const {
    bool ok = true;
    if (field == "server_version") {
        result = string::encodeHtml(getVersionString());
    } else if (field == "server_name") {
        result = string::encodeHtml(Configuration::getInstance().getServerName());
    } else if (field == "server_addr") {
        result = request.getLocalAddress().getAddress();
    } else if (field == "server_port") {
        result = request.getLocalAddress().getPort();
    } else  {
        ok = false;
    }
    return ok;
}

//========================================================================
