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

#include <array>
#include <algorithm> 
#include <functional>

#include "prng.h"

//========================================================================
// prng
//
// Encapsulate the standard MT generator into a convenient singleton
// class. The generator is automatically seeded with an entropy source
// when first accessed.
//========================================================================

//--------------------------------------------------------------
// Private constructor.
//--------------------------------------------------------------

prng::prng() {
    seed();
}

//--------------------------------------------------------------
// Return the singleton instance of this generator.
//--------------------------------------------------------------

prng & prng::instance() {
    static prng singleton;
    return singleton;
}

//--------------------------------------------------------------
// Seed the generator with an entropy source.
//--------------------------------------------------------------

void prng::seed() {
    std::random_device source;
    std::array<uint32_t, std::mt19937::state_size> noise { 0 };
    std::generate(noise.begin(), noise.end(), std::ref(source));
    std::seed_seq seeds(noise.cbegin(), noise.cend());
    generator_.seed(seeds);
}

//--------------------------------------------------------------
// Seed the generator with a single value. (Only used for
// unit testing.)
//--------------------------------------------------------------

void prng::seed(uint32_t value) {
    generator_.seed(value);
}

//========================================================================
