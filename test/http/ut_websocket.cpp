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

#ifdef ZINC_WEBSOCKET

#include "gtest/gtest.h"
#include "http/websocket.h"
#include "../streams.h"

//--------------------------------------------------------------
// Helper function to generate long strings.
//--------------------------------------------------------------

static std::string makeString(size_t n) {
    std::string ret = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    ret.append(n - 26, 'x');
    return ret;
}

//--------------------------------------------------------------
// Fake random number generator.
//--------------------------------------------------------------

class FakePRNG : public iprng {
public:
    FakePRNG()              { reset();              }

    void        reset()     { seed_ = 0x12345678;   }
    uint32_t    next()      { return seed_++;       }

private:
    uint32_t    seed_;
};

//--------------------------------------------------------------
// Test the nonce related fonctions.
//--------------------------------------------------------------

TEST(WebSocket, Nonce) {
    FakePRNG prng;

    std::string n1 = WebSocket::MakeNonce(prng);
    EXPECT_EQ(n1, "eHl6e3x9fn+AgYKDhIWGhw==");
    EXPECT_EQ(WebSocket::TransformNonce(n1), "uTffMBQO++0xHkdF9WvmE2tZX6U=");

    std::string n2 = WebSocket::MakeNonce(prng);
    EXPECT_EQ(n2, "iImKi4yNjo+QkZKTlJWWlw==");
    EXPECT_EQ(WebSocket::TransformNonce(n2), "bfiwkoveO2IpLmiMeZjzT0Q8mEs=");

    std::string n3 = WebSocket::MakeNonce(prng);
    EXPECT_EQ(n3, "mJmam5ydnp+goaKjpKWmpw==");
    EXPECT_EQ(WebSocket::TransformNonce(n3), "D+jvVPgV1YYsmOEtWL5C9IA76hU=");
}

//--------------------------------------------------------------
// Test text messages.
//--------------------------------------------------------------

TEST(WebSocket, TextFrameOut) {
    auto t = [] (std::string const & text, bool masked) -> std::string {
        HexDump os;
        FakePRNG prng;
        WebSocket::Frame frame;
        frame.setTextMessage(text);
        frame.send(os, prng, masked);
        return os.getHexContent(24);
    };

    EXPECT_EQ(t("Hello", false), "81 05 48 65 6C 6C 6F");
    EXPECT_EQ(t("Hello", true),  "81 85 78 56 34 12 30 33 58 7E 17");

    EXPECT_EQ(t(makeString(125),    false),  "81 7D 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53 54 55 56");
    EXPECT_EQ(t(makeString(126),    false),  "81 7E 00 7E 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53 54");
    EXPECT_EQ(t(makeString(127),    false),  "81 7E 00 7F 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53 54");
    EXPECT_EQ(t(makeString(1000),   false),  "81 7E 03 E8 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53 54");
    EXPECT_EQ(t(makeString(1000),   true),   "81 FE 03 E8 78 56 34 12 39 14 77 56 3D 10 73 5A 31 1C 7F 5E 35 18 7B 42");
    EXPECT_EQ(t(makeString(65535),  false),  "81 7E FF FF 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53 54");
    EXPECT_EQ(t(makeString(65536),  false),  "81 7F 00 00 00 00 00 01 00 00 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E");
    EXPECT_EQ(t(makeString(100000), false),  "81 7F 00 00 00 00 00 01 86 A0 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E");
    EXPECT_EQ(t(makeString(200000), true),   "81 FF 00 00 00 00 00 03 0D 40 78 56 34 12 39 14 77 56 3D 10 73 5A 31 1C");
}

//--------------------------------------------------------------
// Test binary messages.
//--------------------------------------------------------------

TEST(WebSocket, BinaryFrameOut) {
    auto t = [] (std::string const & text, bool masked) -> std::string {
        HexDump os;
        FakePRNG prng;
        WebSocket::Frame frame;
        frame.setBinaryMessage(text.data(), text.size());
        frame.send(os, prng, masked);
        return os.getHexContent(24);
    };

    EXPECT_EQ(t("Hello", false), "82 05 48 65 6C 6C 6F");
    EXPECT_EQ(t("Hello", true),  "82 85 78 56 34 12 30 33 58 7E 17");

    EXPECT_EQ(t(makeString(125),    false),  "82 7D 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53 54 55 56");
    EXPECT_EQ(t(makeString(126),    false),  "82 7E 00 7E 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53 54");
    EXPECT_EQ(t(makeString(127),    false),  "82 7E 00 7F 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53 54");
    EXPECT_EQ(t(makeString(1000),   false),  "82 7E 03 E8 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53 54");
    EXPECT_EQ(t(makeString(1000),   true),   "82 FE 03 E8 78 56 34 12 39 14 77 56 3D 10 73 5A 31 1C 7F 5E 35 18 7B 42");
    EXPECT_EQ(t(makeString(65535),  false),  "82 7E FF FF 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53 54");
    EXPECT_EQ(t(makeString(65536),  false),  "82 7F 00 00 00 00 00 01 00 00 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E");
    EXPECT_EQ(t(makeString(100000), false),  "82 7F 00 00 00 00 00 01 86 A0 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E");
    EXPECT_EQ(t(makeString(200000), true),   "82 FF 00 00 00 00 00 03 0D 40 78 56 34 12 39 14 77 56 3D 10 73 5A 31 1C");
}

//--------------------------------------------------------------

#endif

//========================================================================
