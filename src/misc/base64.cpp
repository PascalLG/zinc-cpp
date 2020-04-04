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

#include <array>

#include "base64.h"

//--------------------------------------------------------------
// Encode a block of data in base64.
//--------------------------------------------------------------

std::string base64::encode(void const * data, size_t length) {
    static std::array<char, 64> kEncodingTable = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
        'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
        'w', 'x', 'y', 'z', '0', '1', '2', '3',
        '4', '5', '6', '7', '8', '9', '+', '/'
    };

    std::string ret;
    ret.reserve(4 * ((length + 2) / 3));
    auto src = static_cast<uint8_t const *>(data);

    uint8_t buf[3];
    size_t i = 0;

    while (length--) {
        buf[i++] = *src++;
        if (i == 3) {
            char text[] = {
                kEncodingTable[(buf[0] & 0xfc) >> 2],
                kEncodingTable[((buf[0] & 0x03) << 4) | ((buf[1] & 0xf0) >> 4)],
                kEncodingTable[((buf[1] & 0x0f) << 2) | ((buf[2] & 0xc0) >> 6)],
                kEncodingTable[buf[2] & 0x3f],
            };
            ret.append(text, sizeof(text));
            i = 0;
        }
    }

    if (i) {
        for (size_t j = i; j < 3; j++) {
            buf[j] = 0;
        }
        char text[] = {
            kEncodingTable[(buf[0] & 0xfc) >> 2],
            kEncodingTable[((buf[0] & 0x03) << 4) | ((buf[1] & 0xf0) >> 4)],
            i > 1 ? kEncodingTable[(buf[1] & 0x0f) << 2] : '=',
            '=',
        };
        ret.append(text, sizeof(text));
    }

    return ret;
}

//--------------------------------------------------------------
// Decode a base64 string.
//--------------------------------------------------------------

bool base64::decode(std::vector<uint8_t> & result, std::string const & str) {
    static std::array<uint8_t, 256>  kDecodingTable = {
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
        64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
        64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    };

    size_t length = str.size();
    result.resize(0);
    result.reserve(length * 3 / 4);

    uint32_t buffer[4];
    char const * src = str.data();
    size_t pos = 0, ending = 0;

    while (length--) {
        int ch = static_cast<unsigned char>(*src++);
        if (!isspace(ch)) {
            if (ch == '=') {
                if (pos < 2) {
                    return false;
                }
                buffer[pos++] = 0;
                ending++;
            } else {
                if (ending) {
                    return false;
                }
                uint32_t x = kDecodingTable[ch];
                if (x >= 64) {
                    return false;
                }
                buffer[pos++] = x;
            }
            if (pos == 4) {
                uint32_t triple = (buffer[0] << 18) | (buffer[1] << 12) | (buffer[2] << 6) | buffer[3];
                result.push_back((triple >> 16) & 0xFF);
                if (ending < 2) result.push_back((triple >> 8) & 0xFF);
                if (ending < 1) result.push_back(triple & 0xFF);
                pos = 0;
            }
        }
    }

    return pos == 0;
 }

 //========================================================================
