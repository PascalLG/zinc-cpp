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
#include "misc/logger.h"
#include "http/stream.h"
#include "main/resource_redirection.h"

using namespace std::literals::chrono_literals;

//--------------------------------------------------------------
// Helper class to read a string as a stream.
//--------------------------------------------------------------

class InputString : public InputStream {
public:
    InputString(char const * text, size_t length = 0) : text_(text), offset_(0) {
        length_ = length > 0 ? length : strlen(text);
    }

    size_t read(void * data, size_t length, std::chrono::milliseconds timeout, bool exact) {
        size_t count = 0;
        while (count < length && offset_ < length_) {
            static_cast<char *>(data)[count++] = text_[offset_++];
        }
        return count;
    }

private:
    char const    * text_;
    size_t          offset_;
    size_t          length_;
};

//--------------------------------------------------------------
// Test the getRedirectionStatus function.
//--------------------------------------------------------------

TEST(ResourceRedirection, getRedirectionStatus) {
    auto f = [] (char const * text, bool permanent) {
        InputString src(text);

        HttpRequest req(AddrIPv4(), AddrIPv4(), false);
        EXPECT_TRUE(req.parse(src, 15s, 1024, 8192, 1024 * 1024).isOK());
        return ResourceRedirection("/index.html", permanent).getRedirectionStatus(req);
    };

    logger::setLevel(logger::error, false);
    EXPECT_EQ(f("GET /foo.html HTTP/1.0\r\nHost: example.com\r\n\r\n", false), 302);
    EXPECT_EQ(f("GET /foo.html HTTP/1.0\r\nHost: example.com\r\n\r\n", true),  301);
    EXPECT_EQ(f("GET /foo.html HTTP/1.1\r\nHost: example.com\r\n\r\n", false), 307);
    EXPECT_EQ(f("GET /foo.html HTTP/1.1\r\nHost: example.com\r\n\r\n", true),  308);
}

//--------------------------------------------------------------
// Test the getAbsoluteLocation function.
//--------------------------------------------------------------

TEST(ResourceRedirection, getAbsoluteLocation) {
    auto f = [] (char const * text, char const * location, bool secure) {
        InputString src(text);

        HttpRequest req(AddrIPv4(), AddrIPv4(), secure);
        EXPECT_TRUE(req.parse(src, 15s, 1024, 8192, 1024 * 1024).isOK());
        return ResourceRedirection("/index.html", false).getAbsoluteLocation(req);
    };

    logger::setLevel(logger::error, false);
    EXPECT_EQ(f("GET /foo.html HTTP/1.0\r\nHost: example.com\r\n\r\n", "/index.html", false),  "http://example.com/index.html");
    EXPECT_EQ(f("GET /foo.html HTTP/1.0\r\nHost: example.com\r\n\r\n", "/index.html", true),   "https://example.com/index.html");
    EXPECT_EQ(f("GET /foo.html HTTP/1.0\r\nHost: example.com\r\n\r\n", "index.html", false),   "http://example.com/index.html");
    EXPECT_EQ(f("GET /foo.html HTTP/1.0\r\nHost: example.com\r\n\r\n", "index.html", true),    "https://example.com/index.html");
    EXPECT_EQ(f("GET /foo.html HTTP/1.0\r\nHost: example.com/\r\n\r\n", "/index.html", false), "http://example.com/index.html");
    EXPECT_EQ(f("GET /foo.html HTTP/1.0\r\nHost: example.com/\r\n\r\n", "/index.html", true),  "https://example.com/index.html");
    EXPECT_EQ(f("GET /foo.html HTTP/1.0\r\nHost: example.com/\r\n\r\n", "index.html", false),  "http://example.com/index.html");
    EXPECT_EQ(f("GET /foo.html HTTP/1.0\r\nHost: example.com/\r\n\r\n", "index.html", true),   "https://example.com/index.html");
}

//========================================================================
