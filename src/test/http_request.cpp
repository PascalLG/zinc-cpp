//========================================================================
// Zinc - Unit Testing
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

#include "gtest/gtest.h"
#include "http_request.h"
#include "logger.h"

//--------------------------------------------------------------
// Helper class to read a string as a stream.
//--------------------------------------------------------------

class InputString : public InputStream {
public:
	InputString(char const * text, size_t length = 0) : text_(text), offset_(0) {
		length_ = length > 0 ? length : strlen(text);
	}

    size_t read(void * data, size_t length, int timeout, bool exact) {
    	size_t count = 0;
    	while (count < length && offset_ < length_) {
    		static_cast<char *>(data)[count++] = text_[offset_++];
    	}
    	return count;
    }

private:
	char const    * text_;
	size_t			offset_;
	size_t			length_;
};

//--------------------------------------------------------------
// Test parsing a request (case 1).
//--------------------------------------------------------------

TEST(HttpRequest, Case1) {
	logger::setLevel(logger::error, false);
	InputString src("GET /foo.html HTTP/9.5\r\nAccept: xyz\r\n\r\n");

	HttpRequest req(AddrIn(), AddrIn(), false);
	EXPECT_TRUE(req.parse(src).isOK());

	EXPECT_EQ(req.getVerb(), HttpVerb::GET);
	EXPECT_EQ(req.getURI().getPath(), "/foo.html");
	EXPECT_EQ(req.getHttpVersion(), 0x0905);

    auto const & hdr = req.getHeaders();
    EXPECT_EQ(hdr.size(), 1);
    EXPECT_EQ(hdr.at(HttpHeader::Accept), "xyz");
    EXPECT_EQ(req.getHeaderValue(HttpHeader::Accept), "xyz");
    EXPECT_EQ(req.getHeaderValue(HttpHeader::Host), "");
}

//--------------------------------------------------------------
// Test parsing a request (case 2).
//--------------------------------------------------------------

TEST(HttpRequest, Case2) {
	logger::setLevel(logger::error, false);
	InputString src("POST   /foo.html   \r\n\r\n");

    HttpRequest req(AddrIn(), AddrIn(), false);
    EXPECT_TRUE(req.parse(src).isOK());

	EXPECT_EQ(req.getVerb(), HttpVerb::POST);
	EXPECT_EQ(req.getURI().getPath(), "/foo.html");
	EXPECT_EQ(req.getHttpVersion(), 0x0009);

    auto const & hdr = req.getHeaders();
    EXPECT_EQ(hdr.size(), 0);
    EXPECT_EQ(req.getHeaderValue(HttpHeader::Accept), "");
    EXPECT_EQ(req.getHeaderValue(HttpHeader::Host), "");
}

//--------------------------------------------------------------
// Test parsing a request (case 3).
//--------------------------------------------------------------

TEST(HttpRequest, Case3) {
	logger::setLevel(logger::error, false);
	InputString src("PUT\t/foo.html HTTP/43.12\nAccept:\t abc   \nDate:xyz     \n\n");

    HttpRequest req(AddrIn(), AddrIn(), false);
    EXPECT_TRUE(req.parse(src).isOK());

	EXPECT_EQ(req.getVerb(), HttpVerb::PUT);
	EXPECT_EQ(req.getURI().getPath(), "/foo.html");
	EXPECT_EQ(req.getHttpVersion(), 0x2B0C);

    auto const & hdr = req.getHeaders();
    EXPECT_EQ(hdr.size(), 2);
    EXPECT_EQ(hdr.at(HttpHeader::Accept), "abc");
    EXPECT_EQ(hdr.at(HttpHeader::Date), "xyz");
    EXPECT_EQ(req.getHeaderValue(HttpHeader::Accept), "abc");
    EXPECT_EQ(req.getHeaderValue(HttpHeader::Date), "xyz");
    EXPECT_EQ(req.getHeaderValue(HttpHeader::Host), "");
}

//--------------------------------------------------------------
// Test the getAcceptedEncodings() function.
//--------------------------------------------------------------

TEST(HttpRequest, getAcceptedEncodings) {
	logger::setLevel(logger::error, false);
	InputString src("GET / HTTP/1.1\nAccept-Encoding: gzip, deflate\n\n");

    HttpRequest req(AddrIn(), AddrIn(), false);
    EXPECT_TRUE(req.parse(src).isOK());

    compression::set r = req.getAcceptedEncodings();
    EXPECT_TRUE(r.contains(compression::gzip));
    EXPECT_TRUE(r.contains(compression::deflate));
    EXPECT_FALSE(r.contains(compression::brotli_generic));
    EXPECT_FALSE(r.contains(compression::brotli_text));
    EXPECT_FALSE(r.contains(compression::brotli_font));
}

//========================================================================