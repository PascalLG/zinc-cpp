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

#include <algorithm>

#include "../misc/string.h"

#include "stream_compress.h"
#include "mimetype.h"
#include "compression.h"

//--------------------------------------------------------------
// Table describing the supported compression modes.
//--------------------------------------------------------------

struct Encoding {
    compression::mode                                   mode;
    char const                                        * name;
    std::function<std::unique_ptr<OutputStream>(long)>  factory;
};

static std::initializer_list<Encoding> const encodingTable = {
#if defined(ZINC_COMPRESSION_GZIP)
    { compression::zlib_gzip,       "gzip",     [] (long length) { (void)length; return std::make_unique<StreamDeflate>(true); }          },
#endif
#if defined(ZINC_COMPRESSION_DEFLATE)
    { compression::zlib_deflate,    "deflate",  [] (long length) { (void)length; return std::make_unique<StreamDeflate>(false); }         },
#endif
#if defined(ZINC_COMPRESSION_BROTLI)
    { compression::brotli_generic,  "br",       [] (long length) { return std::make_unique<StreamBrotli>(BROTLI_MODE_GENERIC, length); }  },
    { compression::brotli_text,     "br",       [] (long length) { return std::make_unique<StreamBrotli>(BROTLI_MODE_TEXT, length); }     },
    { compression::brotli_font,     "br",       [] (long length) { return std::make_unique<StreamBrotli>(BROTLI_MODE_FONT, length); }     },
#endif
};

//--------------------------------------------------------------
// Return the normalised encoding name for a given compression 
// mode.
//--------------------------------------------------------------

std::string getCompressionName(compression::mode mode) {
    auto got = std::find_if(encodingTable.begin(), encodingTable.end(), [mode] (Encoding const & x) {
        return x.mode == mode;
    });
    return std::string((got != encodingTable.end()) ? got->name : std::string());
}

//--------------------------------------------------------------
// Parse a list of accepted encodings and return a set of
// compression modes.
//--------------------------------------------------------------

compression::set parseAcceptedEncodings(std::string const & str) {
    compression::set result;

    string::split(str, ',', 0, string::trim_both, [&result] (std::string & name) {
        string::lowercase(name);
        for (Encoding const & x: encodingTable) {
            if (x.name == name) {
                result.insert(x.mode); // do not abort the loop after a match! several modes may have the same name
            }
        }
        return true;
    });

    return result;
}

//--------------------------------------------------------------
// Select the best compression mode, taking into account a set of
// accepted encodings and the mimetype of the data to transfer.
//--------------------------------------------------------------

compression::mode selectCompressionMode(compression::set accepted, Mime const & mimetype) {
    if (!accepted.empty()) {
        compression::mode favorite = mimetype.getFavoriteCompressionMode();

        if (favorite != compression::none) {
            std::initializer_list<compression::mode> favorites = {
                favorite,                       // start with the favorite mode
                compression::brotli_generic,    // then brotli
                compression::zlib_gzip,         // then gzip
                compression::zlib_deflate       // then end with the worst mode
            };
            auto got = std::find_if(favorites.begin(), favorites.end(), [&] (compression::mode m) {
                return accepted.contains(m);
            });
            if (got != favorites.end()) {
                return *got;
            }
        }
    }

    return compression::none;
}

//--------------------------------------------------------------
// Make a stream transformer for the specified compression mode
// and expected data length.
//--------------------------------------------------------------

std::unique_ptr<OutputStream> makeStreamTransformer(compression::mode mode, long length) {
    auto got = std::find_if(encodingTable.begin(), encodingTable.end(), [=] (Encoding const & x) { return x.mode == mode; });
    return (got != encodingTable.end()) ? got->factory(length) : std::unique_ptr<OutputStream>();
}

//========================================================================
