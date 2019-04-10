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

#include <cassert>
#include <algorithm>

#include "misc.h"
#include "http_header.h"

//========================================================================
// HttpHeader
//
// Represent a HTTP header name. For the sake of performance, standard
// header names are encoded by an enum value. It takes less space to
// store and less time to process than a string representation and also
// avoids uppercase/lowercase problems.
// 
// Non standard header names on the other hand are stored as string
// values. Even if this server and usual browsers ignore them, we must
// handle them because they can be used by some CGI scripts and
// webservices.
//========================================================================

//--------------------------------------------------------------
// Constructor from the enumeration.
//--------------------------------------------------------------

HttpHeader::HttpHeader(Code code)
    : code_(code),
      name_() {

    assert(code != UserDefined); // this value is reserved for non standard headers.
}

//--------------------------------------------------------------
// Constructor from a name. (Case insensitive.)
//--------------------------------------------------------------

HttpHeader::HttpHeader(std::string const & name) {
    std::string lowername = name;
    string::lowercase(lowername);
    auto got = getMap().mapByName_.find(lowername);
    if (got == getMap().mapByName_.end()) {
        code_ = UserDefined;
        name_ = name;
    } else {
        code_ = got->second;
        name_ = { };
    }
}

//--------------------------------------------------------------
// Comparison operator (for use with STL containers).
//--------------------------------------------------------------

bool operator == (HttpHeader const & lhs, HttpHeader const & rhs) {
    return (lhs.code_ == rhs.code_) && (lhs.code_ != HttpHeader::UserDefined || string::compare_i(lhs.name_, rhs.name_));
}

//--------------------------------------------------------------
// Compute a hash (for use with STL containers).
//--------------------------------------------------------------

size_t HttpHeader::getHash() const noexcept {
    size_t h = 5381;
    for (char c: name_) {
        h = ((h << 5) + h) ^ (size_t) (unsigned char) tolower(c);
    }
    return code_ ^ (h << 1);
}

//--------------------------------------------------------------
// Return the field name. 
//--------------------------------------------------------------

std::string const & HttpHeader::getFieldName() const {
    if (code_ == UserDefined) {
        return name_;
    } else {
        return getMap().mapByCode_.find(code_)->second;
    }
}

//--------------------------------------------------------------
// Constructor for the Mapping object. This inner class builds
// maps that allow for quick retrieval of a header code from its
// name, or a header name from its code.
//--------------------------------------------------------------

HttpHeader::Mapping::Mapping()
  : mapByCode_ {
        { HttpHeader::AIM,                              "A-IM"                              },
        { HttpHeader::Accept,                           "Accept"                            },
        { HttpHeader::AcceptCharset,                    "Accept-Charset"                    },
        { HttpHeader::AcceptDatetime,                   "Accept-Datetime"                   },
        { HttpHeader::AcceptEncoding,                   "Accept-Encoding"                   },
        { HttpHeader::AcceptLanguage,                   "Accept-Language"                   },
        { HttpHeader::AcceptPatch,                      "Accept-Patch"                      },
        { HttpHeader::AcceptRanges,                     "Accept-Ranges"                     },
        { HttpHeader::AccessControlAllowCredentials,    "Access-Control-Allow-Credentials"  },
        { HttpHeader::AccessControlAllowHeaders,        "Access-Control-Allow-Headers"      },
        { HttpHeader::AccessControlAllowMethods,        "Access-Control-Allow-Methods"      },
        { HttpHeader::AccessControlAllowOrigin,         "Access-Control-Allow-Origin"       },
        { HttpHeader::AccessControlExposeHeaders,       "Access-Control-Expose-Headers"     },
        { HttpHeader::AccessControlMaxAge,              "Access-Control-Max-Age"            },
        { HttpHeader::AccessControlRequestHeaders,      "Access-Control-Request-Headers"    },
        { HttpHeader::AccessControlRequestMethod,       "Access-Control-Request-Method"     },
        { HttpHeader::Age,                              "Age"                               },
        { HttpHeader::Allow,                            "Allow"                             },
        { HttpHeader::AltSvc,                           "Alt-Svc"                           },
        { HttpHeader::Authorization,                    "Authorization"                     },
        { HttpHeader::CacheControl,                     "Cache-Control"                     },
        { HttpHeader::Connection,                       "Connection"                        },
        { HttpHeader::ContentDisposition,               "Content-Disposition"               },
        { HttpHeader::ContentEncoding,                  "Content-Encoding"                  },
        { HttpHeader::ContentLanguage,                  "Content-Language"                  },
        { HttpHeader::ContentLength,                    "Content-Length"                    },
        { HttpHeader::ContentLocation,                  "Content-Location"                  },
        { HttpHeader::ContentMD5,                       "Content-MD5"                       },
        { HttpHeader::ContentRange,                     "Content-Range"                     },
        { HttpHeader::ContentSecurityPolicy,            "Content-Security-Policy"           },
        { HttpHeader::ContentType,                      "Content-Type"                      },
        { HttpHeader::Cookie,                           "Cookie"                            },
        { HttpHeader::DNT,                              "DNT"                               },
        { HttpHeader::Date,                             "Date"                              },
        { HttpHeader::DeltaBase,                        "Delta-Base"                        },
        { HttpHeader::ETag,                             "ETag"                              },
        { HttpHeader::Expect,                           "Expect"                            },
        { HttpHeader::Expires,                          "Expires"                           },
        { HttpHeader::Forwarded,                        "Forwarded"                         },
        { HttpHeader::From,                             "From"                              },
        { HttpHeader::FrontEndHttps,                    "Front-End-Https"                   },
        { HttpHeader::HTTP2Settings,                    "HTTP2-Settings"                    },
        { HttpHeader::Host,                             "Host"                              },
        { HttpHeader::IM,                               "IM"                                },
        { HttpHeader::IfMatch,                          "If-Match"                          },
        { HttpHeader::IfModifiedSince,                  "If-Modified-Since"                 },
        { HttpHeader::IfNoneMatch,                      "If-None-Match"                     },
        { HttpHeader::IfRange,                          "If-Range"                          },
        { HttpHeader::IfUnmodifiedSince,                "If-Unmodified-Since"               },
        { HttpHeader::LastModified,                     "Last-Modified"                     },
        { HttpHeader::Link,                             "Link"                              },
        { HttpHeader::Location,                         "Location"                          },
        { HttpHeader::MaxForwards,                      "Max-Forwards"                      },
        { HttpHeader::Origin,                           "Origin"                            },
        { HttpHeader::P3P,                              "P3P"                               },
        { HttpHeader::Pragma,                           "Pragma"                            },
        { HttpHeader::ProxyAuthenticate,                "Proxy-Authenticate"                },
        { HttpHeader::ProxyAuthorization,               "Proxy-Authorization"               },
        { HttpHeader::ProxyConnection,                  "Proxy-Connection"                  },
        { HttpHeader::PublicKeyPins,                    "Public-Key-Pins"                   },
        { HttpHeader::Range,                            "Range"                             },
        { HttpHeader::Referer,                          "Referer"                           },
        { HttpHeader::Refresh,                          "Refresh"                           },
        { HttpHeader::RetryAfter,                       "Retry-After"                       },
        { HttpHeader::SaveData,                         "Save-Data"                         },
        { HttpHeader::Server,                           "Server"                            },
        { HttpHeader::SetCookie,                        "Set-Cookie"                        },
        { HttpHeader::Status,                           "Status"                            },
        { HttpHeader::StrictTransportSecurity,          "Strict-Transport-Security"         },
        { HttpHeader::TE,                               "TE"                                },
        { HttpHeader::TimingAllowOrigin,                "Timing-Allow-Origin"               },
        { HttpHeader::Tk,                               "Tk"                                },
        { HttpHeader::Trailer,                          "Trailer"                           },
        { HttpHeader::TransferEncoding,                 "Transfer-Encoding"                 },
        { HttpHeader::Upgrade,                          "Upgrade"                           },
        { HttpHeader::UpgradeInsecureRequests,          "Upgrade-Insecure-Requests"         },
        { HttpHeader::UserAgent,                        "User-Agent"                        },
        { HttpHeader::Vary,                             "Vary"                              },
        { HttpHeader::Via,                              "Via"                               },
        { HttpHeader::WWWAuthenticate,                  "WWW-Authenticate"                  },
        { HttpHeader::Warning,                          "Warning"                           },
        { HttpHeader::XATTDeviceId,                     "X-ATT-DeviceId"                    },
        { HttpHeader::XContentDuration,                 "X-Content-Duration"                },
        { HttpHeader::XContentSecurityPolicy,           "X-Content-Security-Policy"         },
        { HttpHeader::XContentTypeOptions,              "X-Content-Type-Options"            },
        { HttpHeader::XCorrelationID,                   "X-Correlation-ID"                  },
        { HttpHeader::XCsrfToken,                       "X-Csrf-Token"                      },
        { HttpHeader::XForwardedFor,                    "X-Forwarded-For"                   },
        { HttpHeader::XForwardedHost,                   "X-Forwarded-Host"                  },
        { HttpHeader::XForwardedProto,                  "X-Forwarded-Proto"                 },
        { HttpHeader::XFrameOptions,                    "X-Frame-Options"                   },
        { HttpHeader::XHttpMethodOverride,              "X-Http-Method-Override"            },
        { HttpHeader::XPoweredBy,                       "X-Powered-By"                      },
        { HttpHeader::XRequestID,                       "X-Request-ID"                      },
        { HttpHeader::XRequestedWith,                   "X-Requested-With"                  },
        { HttpHeader::XUACompatible,                    "X-UA-Compatible"                   },
        { HttpHeader::XUIDH,                            "X-UIDH"                            },
        { HttpHeader::XWapProfile,                      "X-Wap-Profile"                     },
        { HttpHeader::XWebKitCSP,                       "X-WebKit-CSP"                      },
        { HttpHeader::XXSSProtection,                   "X-XSS-Protection"                  },
    },
    mapByName_ {} {

    for (auto & i: mapByCode_) {
        std::string key = i.second;
        string::lowercase(key);
        mapByName_.emplace(key, i.first);
    }
}

//--------------------------------------------------------------
// Return a singleton Mapping object.
//--------------------------------------------------------------

HttpHeader::Mapping & HttpHeader::getMap() const {
    static HttpHeader::Mapping map; // Thread-safe as of C++11
    return map;
}

//========================================================================
