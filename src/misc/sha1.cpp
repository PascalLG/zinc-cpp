//========================================================================
// Zinc - Web Server
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

#include <cassert>
#include <cstring>

#include "sha1.h"

//--------------------------------------------------------------
// Macros.
//--------------------------------------------------------------

#define ROTL(x, n)				(((x) << (n)) | ((x) >> (32 - (n))))

#define P1(a, b, c, d, e, x)	{ e += ROTL(a, 5) + (d ^ (b & (c ^ d))) + 0x5A827999 + x; b = ROTL(b, 30);			}
#define P2(a, b, c, d, e, x)	{ e += ROTL(a, 5) + (b ^ c ^ d) + 0x6ED9EBA1 + x; b = ROTL(b, 30);					}
#define P3(a, b, c, d, e, x)	{ e += ROTL(a, 5) + ((b & c) | (d & (b | c))) + 0x8F1BBCDC + x; b = ROTL(b, 30);	}
#define P4(a, b, c, d, e, x)	{ e += ROTL(a, 5) + (b ^ c ^ d) + 0xCA62C1D6 + x; b = ROTL(b, 30);					}

#define R(t)                    ( tmp = W[(t - 3) & 0x0F] ^ W[(t - 8) & 0x0F] ^ W[(t - 14) & 0x0F] ^ W[(t) & 0x0F], ( W[t & 0x0F] = ROTL(tmp, 1)) )

//========================================================================
// sha1
//
// Implements the SHA-1 hashing function.
//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

digest::sha1::sha1() {
    init();
}

//--------------------------------------------------------------
// Processes a chunk of 64 bytes of data.
//--------------------------------------------------------------

void digest::sha1::process(uint8_t const * data) {
    uint32_t A = state_[0];
    uint32_t B = state_[1];
    uint32_t C = state_[2];
    uint32_t D = state_[3];
    uint32_t E = state_[4];
    uint32_t W[16], tmp;

    for (size_t i = 0; i < 16; i++) {
        W[i] = (static_cast<uint32_t>(data[0]) << 24) | (static_cast<uint32_t>(data[1]) << 16) | (static_cast<uint32_t>(data[2]) << 8) | static_cast<uint32_t>(data[3]);
        data += 4;
    }

    P1(A, B, C, D, E, W[0]);
    P1(E, A, B, C, D, W[1]);
    P1(D, E, A, B, C, W[2]);
    P1(C, D, E, A, B, W[3]);
    P1(B, C, D, E, A, W[4]);
    P1(A, B, C, D, E, W[5]);
    P1(E, A, B, C, D, W[6]);
    P1(D, E, A, B, C, W[7]);
    P1(C, D, E, A, B, W[8]);
    P1(B, C, D, E, A, W[9]);
    P1(A, B, C, D, E, W[10]);
    P1(E, A, B, C, D, W[11]);
    P1(D, E, A, B, C, W[12]);
    P1(C, D, E, A, B, W[13]);
    P1(B, C, D, E, A, W[14]);
    P1(A, B, C, D, E, W[15]);
    P1(E, A, B, C, D, R(16));
    P1(D, E, A, B, C, R(17));
    P1(C, D, E, A, B, R(18));
    P1(B, C, D, E, A, R(19));

    P2(A, B, C, D, E, R(20));
    P2(E, A, B, C, D, R(21));
    P2(D, E, A, B, C, R(22));
    P2(C, D, E, A, B, R(23));
    P2(B, C, D, E, A, R(24));
    P2(A, B, C, D, E, R(25));
    P2(E, A, B, C, D, R(26));
    P2(D, E, A, B, C, R(27));
    P2(C, D, E, A, B, R(28));
    P2(B, C, D, E, A, R(29));
    P2(A, B, C, D, E, R(30));
    P2(E, A, B, C, D, R(31));
    P2(D, E, A, B, C, R(32));
    P2(C, D, E, A, B, R(33));
    P2(B, C, D, E, A, R(34));
    P2(A, B, C, D, E, R(35));
    P2(E, A, B, C, D, R(36));
    P2(D, E, A, B, C, R(37));
    P2(C, D, E, A, B, R(38));
    P2(B, C, D, E, A, R(39));

    P3(A, B, C, D, E, R(40));
    P3(E, A, B, C, D, R(41));
    P3(D, E, A, B, C, R(42));
    P3(C, D, E, A, B, R(43));
    P3(B, C, D, E, A, R(44));
    P3(A, B, C, D, E, R(45));
    P3(E, A, B, C, D, R(46));
    P3(D, E, A, B, C, R(47));
    P3(C, D, E, A, B, R(48));
    P3(B, C, D, E, A, R(49));
    P3(A, B, C, D, E, R(50));
    P3(E, A, B, C, D, R(51));
    P3(D, E, A, B, C, R(52));
    P3(C, D, E, A, B, R(53));
    P3(B, C, D, E, A, R(54));
    P3(A, B, C, D, E, R(55));
    P3(E, A, B, C, D, R(56));
    P3(D, E, A, B, C, R(57));
    P3(C, D, E, A, B, R(58));
    P3(B, C, D, E, A, R(59));

    P4(A, B, C, D, E, R(60));
    P4(E, A, B, C, D, R(61));
    P4(D, E, A, B, C, R(62));
    P4(C, D, E, A, B, R(63));
    P4(B, C, D, E, A, R(64));
    P4(A, B, C, D, E, R(65));
    P4(E, A, B, C, D, R(66));
    P4(D, E, A, B, C, R(67));
    P4(C, D, E, A, B, R(68));
    P4(B, C, D, E, A, R(69));
    P4(A, B, C, D, E, R(70));
    P4(E, A, B, C, D, R(71));
    P4(D, E, A, B, C, R(72));
    P4(C, D, E, A, B, R(73));
    P4(B, C, D, E, A, R(74));
    P4(A, B, C, D, E, R(75));
    P4(E, A, B, C, D, R(76));
    P4(D, E, A, B, C, R(77));
    P4(C, D, E, A, B, R(78));
    P4(B, C, D, E, A, R(79));

    state_[0] += A;
    state_[1] += B;
    state_[2] += C;
    state_[3] += D;
    state_[4] += E;
}

//--------------------------------------------------------------
// Initializes a hash computation.
//--------------------------------------------------------------

void digest::sha1::init() {
    state_[0] = 0x67452301;
    state_[1] = 0xEFCDAB89;
    state_[2] = 0x98BADCFE;
    state_[3] = 0x10325476;
    state_[4] = 0xC3D2E1F0;
    count_ = 0;
}

//--------------------------------------------------------------
// Updates the current hash with the specified data block.
//--------------------------------------------------------------

void digest::sha1::update(void const * data, size_t length) {
    auto ptr = static_cast<uint8_t const *>(data);
    uint64_t rem = count_ & 63;

    if (rem + length >= 64) {
        size_t k = 0;
        if (rem) {
            k = 64 - static_cast<size_t>(rem);
            memcpy(buffer_ + rem, ptr, k);
            process(buffer_);
        }
        for ( ; k + 64 <= length; k += 64) {
            process(ptr + k);
        }
        memcpy(buffer_, ptr + k, length - k);
    } else {
        memcpy(buffer_ + rem, ptr, length);
    }

    count_ += length;
}

//--------------------------------------------------------------
// Finalizes the computation and copies the digest at the
// specified location.
//--------------------------------------------------------------

void digest::sha1::finalize(std::array<uint8_t, 20> & digest) {
    uint8_t	final[80];
    memset(final, 0, 72);

    uint64_t bits = count_ << 3;
    final[79] = static_cast<uint8_t>(bits);
    final[78] = static_cast<uint8_t>(bits >> 8);
    final[77] = static_cast<uint8_t>(bits >> 16);
    final[76] = static_cast<uint8_t>(bits >> 24);
    final[75] = static_cast<uint8_t>(bits >> 32);
    final[74] = static_cast<uint8_t>(bits >> 40);
    final[73] = static_cast<uint8_t>(bits >> 48);
    final[72] = static_cast<uint8_t>(bits >> 56);

    size_t offset = (count_ & 63) + 16;
    if (offset > (55 + 16)) {
        offset -= 64;
    }
    final[offset] = 0x80;

    update(final + offset, 80 - offset);
    assert((count_ & 63) == 0);

    for (size_t i = 0, j = 0; i < 5; i++) {
        digest[j++] = static_cast<uint8_t>(state_[i] >> 24);
        digest[j++] = static_cast<uint8_t>(state_[i] >> 16);
        digest[j++] = static_cast<uint8_t>(state_[i] >> 8);
        digest[j++] = static_cast<uint8_t>(state_[i]);
    }
}

//========================================================================
