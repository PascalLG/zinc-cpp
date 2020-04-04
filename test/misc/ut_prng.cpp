//========================================================================
// Zinc - Unit Testing
// Copyright (c) 2020, Pascal Levy
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

#include <array>
#include <iostream>

#include "gtest/gtest.h"
#include "misc/prng.h"

//--------------------------------------------------------------
// Test the PRNG, sequence 1.
//--------------------------------------------------------------

TEST(PRNG, Sequence1) {
    prng & g = prng::instance();
    g.seed(1234u);

    static std::array<uint32_t, 128> expected = {
        0x31076B2F, 0x7F66E2D3, 0x9F428526, 0xD15DDC35, 0x700EECCC, 0x9CB35D74, 0xC90D4298, 0xC577D7F9,
        0xC7AC7E8F, 0xDC54DAB1, 0x45C8A117, 0x269024E9, 0x46C65C9A, 0x32D2201E, 0xCD477EAB, 0xD0AE849E,
        0xF5489EEC, 0x28A81F7C, 0xE03D1F1A, 0x1DBB3576, 0x5B99E9BA, 0x034DE878, 0x80413770, 0x7CA11DDC,
        0xAEF76D45, 0x54BD6D50, 0xB673A3C9, 0xCD79C97F, 0x5EC8C0F6, 0x192709FE, 0x8FAA8DAF, 0x0E559632,
        0x80CA0EF0, 0x715256FB, 0x0386544C, 0x05AB3925, 0xC5D7F722, 0x4A6D2FA6, 0xE1F4C5FA, 0x3F13B4C3,
        0x5D692AE7, 0xBD005C8B, 0x9D8A9A80, 0xE3A452E9, 0x134C2F4B, 0xFCB52950, 0x5E6B4003, 0x1E10C502,
        0xEEE24513, 0x64CEEB8C, 0xA6C0B7C1, 0x73E619CB, 0x65AF1173, 0x89C40E6B, 0xC9EA37FA, 0xCA6635D1,
        0x511C2C0E, 0x77410CF3, 0x916EE9C7, 0x6F71EF3C, 0xDE7F21EA, 0x91C95A6B, 0x6FA90FAE, 0xF8215B9C,
        0xCD598C51, 0x0A61E357, 0x24CDE70D, 0x8C519074, 0xB44A7260, 0x766B6B8C, 0xB45F70C5, 0x60607BFD,
        0x3802C271, 0x53F20BDF, 0xECC41FE5, 0xD0436E9F, 0x713022F9, 0xA58473FC, 0xE8C8EE59, 0x0C242454,
        0x0F4FA82D, 0xFEB58A10, 0x2F2D7029, 0xB071BF48, 0x0C1F79B8, 0xEDF6B8EC, 0xACC4FF46, 0xEB09AFB8,
        0x98395456, 0xF9AD5FAC, 0x888703DA, 0x65A1EBD3, 0x0B17492F, 0x433B76B1, 0x8FBA1412, 0x6E1E6655,
        0x546526AE, 0xC3B849E2, 0x80C26F25, 0x9987F56D, 0x1CA51B26, 0x14B8F087, 0x9B710BE4, 0xB45D06C3,
        0x90E1BF85, 0x29FCC6EE, 0x01BB4A2F, 0x08480BF6, 0x9E10A8F4, 0x5401A976, 0xE980E2AF, 0x794EE38F,
        0xCA5FCA22, 0x116E000A, 0xFDF90D1C, 0x61F95404, 0xF5740852, 0x1E599067, 0xCABE2959, 0xE575C937,
        0x490634EB, 0xC3A9B94E, 0x9FFA8A97, 0x601A89B2, 0x7A645AF9, 0x069BC23E, 0x3217C4B7, 0xE8126954,
    };

    for (uint32_t x: expected) {
        ASSERT_EQ(x, g.next());
    }
}

//--------------------------------------------------------------
// Test the PRNG, sequence 2.
//--------------------------------------------------------------

TEST(PRNG, Sequence2) {
    prng & g = prng::instance();
    g.seed(56789u);

    static std::array<uint32_t, 128> expected = {
        0xCBE05A0F, 0x88D5FA24, 0xBD3CEEE0, 0x16AFF7CB, 0x09419667, 0x6BA97454, 0xF07A396B, 0xC8B70A55,
        0x2D9643CC, 0x1C01261C, 0x483E97BD, 0x272ADCFA, 0xF676A561, 0x63E0BE1B, 0x0C4598E4, 0x7C584BBA,
        0xA20A800A, 0xC243F2DB, 0x3836DD02, 0x99F0A0D7, 0x4C8E1008, 0xD6B8278E, 0xFC1E66F4, 0x68CD5167,
        0xEA9E70E8, 0x72F91E20, 0x2214373B, 0x7F2D8A81, 0x2A7BFBA4, 0x2F961159, 0xDC93A084, 0x97FC9BB6,
        0x04A7D38A, 0x173916E8, 0x6B40178F, 0x44DC490E, 0x81159EAF, 0x362DFD98, 0xDED0C0CB, 0xB1A9E1B2,
        0x980BAB9E, 0xC25FA1FB, 0x0822B4EA, 0x8E52796D, 0x738AA9FE, 0x4E196FCA, 0x8997070B, 0x56475A4D,
        0xEFE31CF5, 0x5E3DB07A, 0x0BF57F75, 0xD053476F, 0xFF3C7999, 0xDEFC2901, 0x1288EE6A, 0xD1E1B4DD,
        0xE27AC7B1, 0x40BA5E7B, 0x350DAE8D, 0xC51CAEE2, 0xAC731672, 0x915E2EE0, 0x3050E290, 0x67CE7651,
        0x4BF73555, 0xE48936D7, 0x6DAAE689, 0x9CB3A791, 0xF40392FA, 0xC1D37FA5, 0x13284090, 0xC4C60596,
        0xE350A597, 0x9D88F323, 0xF5CD1EAE, 0x8394EC48, 0x38B91D03, 0x9B392E6A, 0xED6A8574, 0x8C52DF1F,
        0x74B4B259, 0x3C9C5039, 0xABA15618, 0xD9B25606, 0x67BBECF3, 0x26A3EAD3, 0xDD85BF52, 0xA7B8F4A1,
        0xD8BE0F54, 0x0FA86A68, 0xB1AA53F4, 0xD21D48D1, 0x44C8430F, 0xB6BCF3F9, 0x9D768E3E, 0xE217A1B6,
        0x410E2423, 0x39206DE5, 0x9ED633CF, 0xF958EBDC, 0x5805A06C, 0x0F561750, 0x6DCF8E7A, 0xA02B35E1,
        0x28514AF0, 0x09F0EE99, 0x8C0B2EA7, 0x9F699731, 0xF5129E42, 0x5332DE50, 0xFB1D3754, 0x357C7681,
        0x809FDD2E, 0x08FFE150, 0x1AADD862, 0x3A330E24, 0xAE99E729, 0xB270BBD6, 0xC674C01D, 0xAFF9C413,
        0x03A2C548, 0x59CFF8D6, 0xCE8BC1FF, 0x02DB4C2D, 0xEF884378, 0x866C96E6, 0x19ACC02A, 0xDD1D34D7,        
    };

    for (uint32_t x: expected) {
        ASSERT_EQ(x, g.next());
    }
}

//--------------------------------------------------------------
// Test the statistical distribution.
//--------------------------------------------------------------

TEST(PRNG, Statistics) {
    prng & g = prng::instance();
    g.seed();

    const int N = 1000000;
    std::array<unsigned int, 256> histo = { 0 };

    for (size_t i = 0; i < N / 4; i++) {
        uint32_t v = g.next();
        histo[v & 255]++;
        histo[(v >> 8) & 255]++;
        histo[(v >> 16) & 255]++;
        histo[(v >> 24) & 255]++;
    }

    double t = 0.0, s = 0.0;
    for (size_t i = 0; i < 256; i++) {
        double x = static_cast<double>(histo[i]);
        s += i * x;
        t += x * x;
    }

    double average = s / N;
    EXPECT_GE(average, 127.25);
    EXPECT_LE(average, 128.75);

    double khi = (256.0 * t / N) - N;
    EXPECT_GE(khi, 256.0 - 2.0 * sqrt(256.0));  // may fail sometimes...
    EXPECT_LE(khi, 256.0 + 2.0 * sqrt(256.0));
}

//========================================================================
