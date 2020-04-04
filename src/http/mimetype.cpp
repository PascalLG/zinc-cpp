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
#include "../misc/logger.h"
#include "mimetype.h"

//--------------------------------------------------------------
// Table of supported MIME types.
//--------------------------------------------------------------

struct MimeType {
    char const        * extension;     // file extension
    char const        * mime;          // corresponding MIME type
    compression::mode   compression;   // recommended compression mode
};

static std::initializer_list<MimeType> mimeTypes = {
    { "aac",    "audio/aac",                                                                 compression::none              },
    { "abw",    "application/x-abiword",                                                     compression::none              },
    { "avi",    "video/x-msvideo",                                                           compression::none              },
    { "azw",    "application/vnd.amazon.ebook",                                              compression::none              },
    { "bz",     "application/x-bzip",                                                        compression::none              },
    { "bz2",    "application/x-bzip2",                                                       compression::none              },
    { "csh",    "application/x-csh",                                                         compression::none              },
    { "css",    "text/css",                                                                  compression::brotli_text       },
    { "csv",    "text/csv",                                                                  compression::brotli_text       },
    { "doc",    "application/msword",                                                        compression::brotli_generic    },
    { "docx",   "application/vnd.openxmlformats-officedocument.wordprocessingml.document",   compression::none              },
    { "eot",    "application/vnd.ms-fontobject",                                             compression::none              },
    { "epub",   "application/epub+zip",                                                      compression::none              },
    { "gif",    "image/gif",                                                                 compression::none              },
    { "htm",    "text/html",                                                                 compression::brotli_text       },
    { "html",   "text/html",                                                                 compression::brotli_text       },
    { "ico",    "image/x-icon",                                                              compression::none              },
    { "ics",    "text/calendar",                                                             compression::none              },
    { "jar",    "application/java-archive",                                                  compression::none              },
    { "jpeg",   "image JPEG image/jpeg",                                                     compression::none              },
    { "jpg",    "image JPEG image/jpeg",                                                     compression::none              },
    { "js",     "application/javascript",                                                    compression::brotli_text       },
    { "json",   "application/json",                                                          compression::brotli_text       },
    { "mid",    "audio/midi",                                                                compression::none              },
    { "midi",   "audio/midi",                                                                compression::none              },
    { "mpeg",   "video/mpeg",                                                                compression::none              },
    { "mpkg",   "application/vnd.apple.installer+xml",                                       compression::none              },
    { "odp",    "application/vnd.oasis.opendocument.presentation",                           compression::none              },
    { "ods",    "application/vnd.oasis.opendocument.spreadsheet",                            compression::none              },
    { "odt",    "application/vnd.oasis.opendocument.text",                                   compression::none              },
    { "oga",    "audio/ogg",                                                                 compression::none              },
    { "ogv",    "video/ogg",                                                                 compression::none              },
    { "ogx",    "application/ogg",                                                           compression::none              },
    { "otf",    "font/otf",                                                                  compression::brotli_generic    },
    { "png",    "image/png",                                                                 compression::none              },
    { "pdf",    "application/pdf",                                                           compression::none              },
    { "ppt",    "application/vnd.ms-powerpoint",                                             compression::none              },
    { "pptx",   "application/vnd.openxmlformats-officedocument.presentationml.presentation", compression::none              },
    { "rar",    "application/x-rar-compressed",                                              compression::none              },
    { "rtf",    "application/rtf",                                                           compression::brotli_text       },
    { "sh",     "application/x-sh",                                                          compression::none              },
    { "svg",    "image/svg+xml",                                                             compression::none              },
    { "swf",    "application/x-shockwave-flash",                                             compression::none              },
    { "tar",    "application/x-tar",                                                         compression::none              },
    { "tif",    "image/tiff",                                                                compression::none              },
    { "tiff",   "image/tiff",                                                                compression::none              },
    { "ts",     "application/typescript",                                                    compression::none              },
    { "ttf",    "font/ttf",                                                                  compression::none              },
    { "txt",    "text/plain",                                                                compression::brotli_text       },
    { "vsd",    "application/vnd.visio",                                                     compression::none              },
    { "wav",    "audio/x-wav",                                                               compression::none              },
    { "weba",   "audio/webm",                                                                compression::none              },
    { "webm",   "video/webm",                                                                compression::none              },
    { "webp",   "image/webp",                                                                compression::none              },
    { "woff",   "font/woff",                                                                 compression::none              },
    { "woff2",  "font/woff2",                                                                compression::brotli_font       },
    { "xhtml",  "application/xhtml+xml",                                                     compression::brotli_text       },
    { "xls",    "application/vnd.ms-excel",                                                  compression::none              },
    { "xlsx",   "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",         compression::none              },
    { "xml",    "application/xml",                                                           compression::brotli_text       },
    { "xul",    "application/vnd.mozilla.xul+xml",                                           compression::none              },
    { "zip",    "application/zip",                                                           compression::none              },
    { "3gp",    "video/3gpp",                                                                compression::none              },
    { "3g2",    "video/3gpp2",                                                               compression::none              },
    { "7z",     "application/x-7z-compressed",                                               compression::none              },
};

//========================================================================
// MIME types related functions.
//========================================================================

//--------------------------------------------------------------
// Determine the MIME type of a file from its extension and for 
// text files, from its content.
//--------------------------------------------------------------

std::string getMimeType(fs::filepath const & filename, std::istream * content) {
    std::string result;
    std::string ext = filename.getExtension();
    if (ext.length() > 1 && ext.front() == '.') {
        string::lowercase(ext);
        auto got = std::find_if(mimeTypes.begin(), mimeTypes.end(), [&] (MimeType const & mt) { return !ext.compare(1, std::string::npos, mt.extension); });
        if (got != mimeTypes.end()) {
            result = got->mime;
        }
    }

    if (content != nullptr && (result.empty() || result.compare(0, 5, "text/") == 0)) {
        LOG_TRACE("Guessing encoding for " << filename);
        std::string enc = guessEncoding(*content);
        if (!enc.empty()) {
            LOG_TRACE("Guessed encoding: " << enc);
            if (result.empty()) {
                result = "text/plain";
            }
            result += "; charset=";
            result += enc;
        }
    }

    if (result.empty()) {
        result = "application/octet-stream";
    }

    return result;
}

//--------------------------------------------------------------
// Return the best compression mode for a given type. Basically,
// we don't bother compressing files that are already compressed,
// such as jpeg ou zip, because we know in advance it does not
// worth it.
//--------------------------------------------------------------

compression::mode getFavoriteCompressionMode(std::string const & mimetype) {
    size_t off = mimetype.find(';');
    if (off != std::string::npos) {
        while (off > 0 && isblank(mimetype[off - 1])) {
            off--;
        }
    }
    auto got = std::find_if(mimeTypes.begin(), mimeTypes.end(), [&] (MimeType const & mt) { return !mimetype.compare(0, off, mt.mime); });
    return (got != mimeTypes.end()) ? got->compression : compression::none;
}

//========================================================================
// Text encoding functions.
//========================================================================

//--------------------------------------------------------------
// Guess the encoding of a data stream. Only trivial cases are
// supported: UTF16 with BOM, UTF8 and ASCII. Return an empty
// string if the encoding could not be determined.
//--------------------------------------------------------------

std::string guessEncoding(std::istream & is) {
    std::string result;

    char buffer[2048]; // only test the first 2 kb of the file
    is.seekg(0, std::istream::end);
    size_t count = std::min(static_cast<size_t>(is.tellg()), sizeof(buffer));
    is.seekg(0, std::istream::beg);
    is.read(buffer, count);

    if (isUTF16(buffer, count, false)) {
        result = "utf-16le";
    } else if (isUTF16(buffer, count, true)) {
        result = "utf-16be";
    } else {
        bool ascii;
        if (isUTF8(buffer, count, &ascii)) {
            result = ascii ? "ascii" : "utf-8";
        }
    }

    return result;
}

//--------------------------------------------------------------
// Check if a buffer contains valid UTF-8 data. Also return
// a flag indicating if the decoded text only contains ASCII
// characters.
//--------------------------------------------------------------

bool isUTF8(char const * text, size_t length, bool * ascii) {
    auto str = reinterpret_cast<uint8_t const *>(text);
    size_t ndx = 0;
    *ascii = true;

    while (ndx < length) {
        size_t bytecount;
        uint32_t ch = str[ndx];
        if (ch <= 0x7F) {
            bytecount = 1;
        } else {
            *ascii = false;
            if ((ch & 0xE0) == 0xC0) {
                bytecount = 2;
                ch &= 0x1F;
            } else if ((ch & 0xF0) == 0xE0) {
                bytecount = 3;
                ch &= 0x0F;
            } else if ((ch & 0xF8) == 0xF0) {
                bytecount = 4;
                ch &= 0x07;
            } else {
                return false;
            }
        }

        if (ndx + bytecount <= length) {
            for (size_t i = 1; i < bytecount; i++) {
                uint32_t c = str[ndx + i];
                if ((c & 0xC0) != 0x80) {
                    return false;
                }
                ch = (ch << 6) | (c & 0x3F);
            }
            if (!isValidUnicodeChar(ch) || (ch == 0xFEFF && ndx != 0)) {
                return false;
            } 
            if (bytecount == 3) {
                if (ch < 0x0800 || (ch >> 11) == 0x1B) {
                    return false;
                }
            } else if (bytecount == 4) {
                if (ch < 0x10000 || ch > 0x10FFFF) {
                    return false;
                }
            }
        }
        ndx += bytecount;
    }

    return true;
}

//--------------------------------------------------------------
// Try to guess if a buffer contains valid UTF16 data with the
// specified endianness. We simply check the size is even and 
// that we have at least a few printable ASCII chars or a few
// frequently used control characters.
//--------------------------------------------------------------

bool isUTF16(char const * text, size_t length, bool bigendian) {
    int ascii = 0;
    if ((length & 1) == 0) {
        size_t pos = bigendian ? 1 : 0;
        for (size_t i = 0; i < length; i += 2) {
            unsigned ch = static_cast<unsigned>(static_cast<uint8_t>(text[i + pos])) | (static_cast<unsigned>(static_cast<uint8_t>(text[i + 1 - pos])) << 8);
            if (!isValidUnicodeChar(ch) || (ch == 0xFEFF && i != 0)) {
                return false;
            }
            if (ch < 127) {
                ascii++;
            }
        }
    }
    return (ascii >= 5);
}

//--------------------------------------------------------------
// Check if a value corresponds to a valid unicode character.
//--------------------------------------------------------------

bool isValidUnicodeChar(uint32_t ch) {
    // check if control character
    static uint32_t const ctrl = (1u << '\a') | (1u << '\b') | (1u << '\t') | (1u << '\n') |
                                 (1u << '\v') | (1u << '\f') | (1u << '\r') | (1u << '\x1B');

    if (ch < 32 && ((1u << ch) & ctrl) != 0) {
        return true;
    }

    // check if BMP or SMP or SIP
    if (ch >= 0x20 && ch <= 0x2FA1F) {
        return true;
    }

    // check if SSP
    if (ch >= 0xE0000 && ch <= 0xE01EF) {
        return true;
    }

    // check private areas
    if (ch >= 0xF0000 && ch <= 0x10FFFF) {
        return true;
    }

    return false;
}

//========================================================================
