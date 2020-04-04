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
#include "http/http_verb.h"

//--------------------------------------------------------------
// Test the HttpVerb class creation and comparison.
//--------------------------------------------------------------

TEST(HttpVerb, Creation) {
    EXPECT_EQ(HttpVerb("GET"),      HttpVerb::Get       );
    EXPECT_EQ(HttpVerb("HEAD"),     HttpVerb::Head      );
    EXPECT_EQ(HttpVerb("POST"),     HttpVerb::Post      );
    EXPECT_EQ(HttpVerb("PUT"),      HttpVerb::Put       );
    EXPECT_EQ(HttpVerb("DELETE"),   HttpVerb::Delete    );
    EXPECT_EQ(HttpVerb("CONNECT"),  HttpVerb::Connect   );
    EXPECT_EQ(HttpVerb("OPTIONS"),  HttpVerb::Options   );
    EXPECT_EQ(HttpVerb("TRACE"),    HttpVerb::Trace     );
    EXPECT_EQ(HttpVerb("PATCH"),    HttpVerb::Patch     );

    EXPECT_FALSE(HttpVerb().isValid());
    EXPECT_FALSE(HttpVerb("get").isValid());
    EXPECT_FALSE(HttpVerb("ABCDEF").isValid());
    EXPECT_TRUE (HttpVerb(HttpVerb::Post).isValid());
    EXPECT_TRUE (HttpVerb("POST").isValid());

    EXPECT_FALSE(HttpVerb() == HttpVerb());
}

//--------------------------------------------------------------
// Test the HttpVerb class creation and comparison.
//--------------------------------------------------------------

TEST(HttpVerb, Name) {
    EXPECT_EQ(HttpVerb().getVerbName(),                  ""         );
    EXPECT_EQ(HttpVerb(HttpVerb::Get).getVerbName(),     "GET"      );
    EXPECT_EQ(HttpVerb(HttpVerb::Head).getVerbName(),    "HEAD"     );
    EXPECT_EQ(HttpVerb(HttpVerb::Post).getVerbName(),    "POST"     );
    EXPECT_EQ(HttpVerb(HttpVerb::Put).getVerbName(),     "PUT"      );
    EXPECT_EQ(HttpVerb(HttpVerb::Delete).getVerbName(),  "DELETE"   );
    EXPECT_EQ(HttpVerb(HttpVerb::Connect).getVerbName(), "CONNECT"  );
    EXPECT_EQ(HttpVerb(HttpVerb::Options).getVerbName(), "OPTIONS"  );
    EXPECT_EQ(HttpVerb(HttpVerb::Trace).getVerbName(),   "TRACE"    );
    EXPECT_EQ(HttpVerb(HttpVerb::Patch).getVerbName(),   "PATCH"    );
}

//--------------------------------------------------------------
// Test the HttpVerb set operation.
//--------------------------------------------------------------

TEST(HttpVerb, Set) {
    HttpVerb v1("GET"), v2("PUT"), v3;

    EXPECT_TRUE (v1.isOneOf(HttpVerb::Get       ));
    EXPECT_FALSE(v1.isOneOf(HttpVerb::Head      ));
    EXPECT_FALSE(v1.isOneOf(HttpVerb::Post      ));
    EXPECT_FALSE(v1.isOneOf(HttpVerb::Put       ));
    EXPECT_FALSE(v1.isOneOf(HttpVerb::Delete    ));
    EXPECT_FALSE(v1.isOneOf(HttpVerb::Connect   ));
    EXPECT_FALSE(v1.isOneOf(HttpVerb::Options   ));
    EXPECT_FALSE(v1.isOneOf(HttpVerb::Trace     ));
    EXPECT_FALSE(v1.isOneOf(HttpVerb::Patch     ));

    EXPECT_FALSE(v2.isOneOf(HttpVerb::Get       ));
    EXPECT_FALSE(v2.isOneOf(HttpVerb::Head      ));
    EXPECT_FALSE(v2.isOneOf(HttpVerb::Post      ));
    EXPECT_TRUE (v2.isOneOf(HttpVerb::Put       ));
    EXPECT_FALSE(v2.isOneOf(HttpVerb::Delete    ));
    EXPECT_FALSE(v2.isOneOf(HttpVerb::Connect   ));
    EXPECT_FALSE(v2.isOneOf(HttpVerb::Options   ));
    EXPECT_FALSE(v2.isOneOf(HttpVerb::Trace     ));
    EXPECT_FALSE(v2.isOneOf(HttpVerb::Patch     ));

    EXPECT_FALSE(v3.isOneOf(HttpVerb::Get       ));
    EXPECT_FALSE(v3.isOneOf(HttpVerb::Head      ));
    EXPECT_FALSE(v3.isOneOf(HttpVerb::Post      ));
    EXPECT_FALSE(v3.isOneOf(HttpVerb::Put       ));
    EXPECT_FALSE(v3.isOneOf(HttpVerb::Delete    ));
    EXPECT_FALSE(v3.isOneOf(HttpVerb::Connect   ));
    EXPECT_FALSE(v3.isOneOf(HttpVerb::Options   ));
    EXPECT_FALSE(v3.isOneOf(HttpVerb::Trace     ));
    EXPECT_FALSE(v3.isOneOf(HttpVerb::Patch     ));

    EXPECT_TRUE (v1.isOneOf(HttpVerb::Get | HttpVerb::Head | HttpVerb::Post));
    EXPECT_FALSE(v1.isOneOf(HttpVerb::Head | HttpVerb::Post | HttpVerb::Put));
}

//========================================================================
