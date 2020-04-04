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

#ifndef PRNG_H
#define PRNG_H

#include <cstdint>
#include <random>

//--------------------------------------------------------------
// Interface for a random number generator.
//--------------------------------------------------------------

class iprng {
public:
    virtual ~iprng() = default;

    virtual uint32_t next() = 0;
};

//--------------------------------------------------------------
// A simple random number generator.
//--------------------------------------------------------------

class prng : public iprng {
public:
    static prng & instance();

    void        seed();
    void        seed(uint32_t value);
    uint32_t    next() override             { return generator_(); }

private:
    prng();
    prng(prng const &) = delete;

    std::mt19937    generator_;
};

//--------------------------------------------------------------

#endif

//========================================================================
