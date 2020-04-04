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

#include <sstream>
#include <iomanip>

#include "streams.h"

//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

HexDump::HexDump() {
}

//--------------------------------------------------------------
// Write data to the stream.
//--------------------------------------------------------------

bool HexDump::write(void const * data, size_t length) {
    uint8_t const * src = static_cast<uint8_t const *>(data);
    data_.insert(data_.end(), src, src + length);
    return true;
}

//--------------------------------------------------------------

void HexDump::emitHeaders(long length) {
    std::ostringstream oss;
    if (length >= 0) {
        oss << "Length: " << length;
    } else {
        oss << "Chunked";
    }
    oss << "|";
    std::string s = oss.str();
    data_.insert(data_.end(), s.cbegin(), s.cend());
}

//--------------------------------------------------------------

void HexDump::reset() {
    data_.clear();
}

//--------------------------------------------------------------
// Return a textual representation of the stream content.
//--------------------------------------------------------------

std::string HexDump::getRawContent() const {
    return std::string(data_.cbegin(), data_.cend());
}

//--------------------------------------------------------------
// Return a textual representation of the stream content.
//--------------------------------------------------------------

std::string HexDump::getHexContent(size_t n) const {
    std::ostringstream oss;
    oss.flags(std::ios::hex | std::ios::right | std::ios::uppercase);
    oss.fill('0');
    if (n == 0 || n > data_.size()) {
        n = data_.size();
    }
    for (size_t i = 0; i < n; i++) {
        oss << std::setw(2) << static_cast<unsigned>(data_[i]) << ' ';
    }
    std::string ret = oss.str();
    if (!ret.empty() && ret.back() == ' ') {
        ret.pop_back();
    }
    return ret;
}

//========================================================================
