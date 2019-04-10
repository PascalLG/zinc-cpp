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
#include "http_verb.h"

//--------------------------------------------------------------
// Test the HttpVerb class creation and comparison.
//--------------------------------------------------------------

TEST(HttpVerb, Creation) {
    EXPECT_EQ(HttpVerb("GET"),      HttpVerb::GET       );
    EXPECT_EQ(HttpVerb("HEAD"),     HttpVerb::HEAD      );
    EXPECT_EQ(HttpVerb("POST"),     HttpVerb::POST      );
    EXPECT_EQ(HttpVerb("PUT"),      HttpVerb::PUT       );
    EXPECT_EQ(HttpVerb("DELETE"),   HttpVerb::DELETE    );
    EXPECT_EQ(HttpVerb("CONNECT"),  HttpVerb::CONNECT   );
    EXPECT_EQ(HttpVerb("OPTIONS"),  HttpVerb::OPTIONS   );
    EXPECT_EQ(HttpVerb("TRACE"),    HttpVerb::TRACE     );
    EXPECT_EQ(HttpVerb("PATCH"),    HttpVerb::PATCH     );

    EXPECT_FALSE(HttpVerb().isValid());
    EXPECT_FALSE(HttpVerb("get").isValid());
    EXPECT_FALSE(HttpVerb("ABCDEF").isValid());
    EXPECT_TRUE (HttpVerb(HttpVerb::POST).isValid());
    EXPECT_TRUE (HttpVerb("POST").isValid());

    EXPECT_FALSE(HttpVerb() == HttpVerb());
}

//--------------------------------------------------------------
// Test the HttpVerb class creation and comparison.
//--------------------------------------------------------------

TEST(HttpVerb, Name) {
    EXPECT_EQ(HttpVerb().getVerbName(),                  ""         );
    EXPECT_EQ(HttpVerb(HttpVerb::GET).getVerbName(),     "GET"      );
    EXPECT_EQ(HttpVerb(HttpVerb::HEAD).getVerbName(),    "HEAD"     );
    EXPECT_EQ(HttpVerb(HttpVerb::POST).getVerbName(),    "POST"     );
    EXPECT_EQ(HttpVerb(HttpVerb::PUT).getVerbName(),     "PUT"      );
    EXPECT_EQ(HttpVerb(HttpVerb::DELETE).getVerbName(),  "DELETE"   );
    EXPECT_EQ(HttpVerb(HttpVerb::CONNECT).getVerbName(), "CONNECT"  );
    EXPECT_EQ(HttpVerb(HttpVerb::OPTIONS).getVerbName(), "OPTIONS"  );
    EXPECT_EQ(HttpVerb(HttpVerb::TRACE).getVerbName(),   "TRACE"    );
    EXPECT_EQ(HttpVerb(HttpVerb::PATCH).getVerbName(),   "PATCH"    );
}

//--------------------------------------------------------------
// Test the HttpVerb set operation.
//--------------------------------------------------------------

TEST(HttpVerb, Set) {
    HttpVerb v1("GET"), v2("PUT"), v3;

    EXPECT_TRUE (v1.isOneOf(HttpVerb::GET       ));
    EXPECT_FALSE(v1.isOneOf(HttpVerb::HEAD      ));
    EXPECT_FALSE(v1.isOneOf(HttpVerb::POST      ));
    EXPECT_FALSE(v1.isOneOf(HttpVerb::PUT       ));
    EXPECT_FALSE(v1.isOneOf(HttpVerb::DELETE    ));
    EXPECT_FALSE(v1.isOneOf(HttpVerb::CONNECT   ));
    EXPECT_FALSE(v1.isOneOf(HttpVerb::OPTIONS   ));
    EXPECT_FALSE(v1.isOneOf(HttpVerb::TRACE     ));
    EXPECT_FALSE(v1.isOneOf(HttpVerb::PATCH     ));

    EXPECT_FALSE(v2.isOneOf(HttpVerb::GET       ));
    EXPECT_FALSE(v2.isOneOf(HttpVerb::HEAD      ));
    EXPECT_FALSE(v2.isOneOf(HttpVerb::POST      ));
    EXPECT_TRUE (v2.isOneOf(HttpVerb::PUT       ));
    EXPECT_FALSE(v2.isOneOf(HttpVerb::DELETE    ));
    EXPECT_FALSE(v2.isOneOf(HttpVerb::CONNECT   ));
    EXPECT_FALSE(v2.isOneOf(HttpVerb::OPTIONS   ));
    EXPECT_FALSE(v2.isOneOf(HttpVerb::TRACE     ));
    EXPECT_FALSE(v2.isOneOf(HttpVerb::PATCH     ));

    EXPECT_FALSE(v3.isOneOf(HttpVerb::GET       ));
    EXPECT_FALSE(v3.isOneOf(HttpVerb::HEAD      ));
    EXPECT_FALSE(v3.isOneOf(HttpVerb::POST      ));
    EXPECT_FALSE(v3.isOneOf(HttpVerb::PUT       ));
    EXPECT_FALSE(v3.isOneOf(HttpVerb::DELETE    ));
    EXPECT_FALSE(v3.isOneOf(HttpVerb::CONNECT   ));
    EXPECT_FALSE(v3.isOneOf(HttpVerb::OPTIONS   ));
    EXPECT_FALSE(v3.isOneOf(HttpVerb::TRACE     ));
    EXPECT_FALSE(v3.isOneOf(HttpVerb::PATCH     ));

    EXPECT_TRUE (v1.isOneOf(HttpVerb::GET | HttpVerb::HEAD | HttpVerb::POST));
    EXPECT_FALSE(v1.isOneOf(HttpVerb::HEAD | HttpVerb::POST | HttpVerb::PUT));
}

//========================================================================
