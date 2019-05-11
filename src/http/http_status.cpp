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

#include "../misc/string.h"
#include "http_status.h"

//========================================================================
// HttpStatus
//
// Represent an HTTP status code. Provide methods to store, compare and
// manipulate HTTP statuses.
//========================================================================

//--------------------------------------------------------------
// Return the string associated to a given HTTP status.
//--------------------------------------------------------------

std::string const & HttpStatus::getStatusString() const {
    auto const & map = getMap().mapByStatus_;
    auto got = map.find(status_);
    return got != map.end() ? got->second : string::empty;
}

//--------------------------------------------------------------
// Constructor for the Mapping object.
//--------------------------------------------------------------

HttpStatus::Mapping::Mapping()
  : mapByStatus_ {
        { 100,  "Continue"                              },
        { 101,  "Switching Protocols"                   },
        { 102,  "Processing"                            },
        { 103,  "Early Hints"                           },
        { 200,  "OK"                                    },
        { 201,  "Created"                               },
        { 202,  "Accepted"                              },
        { 203,  "Non-Authoritative Information"         },
        { 204,  "No Content"                            },
        { 205,  "Reset Content"                         },
        { 206,  "Partial Content"                       },
        { 207,  "Multi-Status"                          },
        { 208,  "Already Reported"                      },
        { 210,  "Content Different"                     },
        { 226,  "IM Used"                               },
        { 300,  "Multiple Choices"                      },
        { 301,  "Moved Permanently"                     },
        { 302,  "Found"                                 },
        { 303,  "See Other"                             },
        { 304,  "Not Modified"                          },
        { 305,  "Use Proxy"                             },
        { 306,  "Switch Proxy"                          },
        { 307,  "Temporary Redirect"                    },
        { 308,  "Permanent Redirect"                    },
        { 310,  "Too many Redirects"                    },
        { 400,  "Bad Request"                           },
        { 401,  "Unauthorized"                          },
        { 402,  "Payment Required"                      },
        { 403,  "Forbidden"                             },
        { 404,  "Not Found"                             },
        { 405,  "Method Not Allowed"                    },
        { 406,  "Not Acceptable"                        },
        { 407,  "Proxy Authentication Required"         },
        { 408,  "Request Time-out"                      },
        { 409,  "Conflict"                              },
        { 410,  "Gone"                                  },
        { 411,  "Length Required"                       },
        { 412,  "Precondition Failed"                   },
        { 413,  "Request Entity Too Large"              },
        { 414,  "Request-URI Too Long"                  },
        { 415,  "Unsupported Media Type"                },
        { 416,  "Requested range unsatisfiable"         },
        { 417,  "Expectation failed"                    },
        { 418,  "I’m a teapot"                          },
        { 421,  "Bad mapping / Misdirected Request"     },
        { 422,  "Unprocessable entity"                  },
        { 423,  "Locked"                                },
        { 424,  "Method failure"                        },
        { 425,  "Unordered Collection"                  },
        { 426,  "Upgrade Required"                      },
        { 428,  "Precondition Required"                 },
        { 429,  "Too Many Requests"                     },
        { 431,  "Request Header Fields Too Large"       },
        { 449,  "Retry With"                            },
        { 450,  "Blocked by Windows Parental Controls"  },
        { 451,  "Unavailable For Legal Reasons"         },
        { 456,  "Unrecoverable Error"                   },
        { 500,  "Internal Server Error"                 },
        { 501,  "Not Implemented"                       },
        { 502,  "Bad Gateway"                           },
        { 503,  "Service Unavailable"                   },
        { 504,  "Gateway Time-out"                      },
        { 505,  "HTTP Version not supported"            },
        { 506,  "Variant Also Negotiates"               },
        { 507,  "Insufficient storage"                  },
        { 508,  "Loop detected"                         },
        { 509,  "Bandwidth Limit Exceeded"              },
        { 510,  "Not extended"                          },
        { 511,  "Network authentication required"       },
    } {
}

//--------------------------------------------------------------
// Return a singleton Mapping object.
//--------------------------------------------------------------

HttpStatus::Mapping & HttpStatus::getMap() const {
    static HttpStatus::Mapping map; // Thread-safe as of C++11
    return map;
}

//========================================================================
