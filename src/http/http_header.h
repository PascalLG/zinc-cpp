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

#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H

#include <string>
#include <unordered_map>

//--------------------------------------------------------------
// HTTP header field name.
//--------------------------------------------------------------

class HttpHeader {
public:
    enum Code {
        UserDefined,
        AIM,
        Accept,
        AcceptCharset,
        AcceptDatetime,
        AcceptEncoding,
        AcceptLanguage,
        AcceptPatch,
        AcceptRanges,
        AccessControlAllowCredentials,
        AccessControlAllowHeaders,
        AccessControlAllowMethods,
        AccessControlAllowOrigin,
        AccessControlExposeHeaders,
        AccessControlMaxAge,
        AccessControlRequestHeaders,
        AccessControlRequestMethod,
        Age,
        Allow,
        AltSvc,
        Authorization,
        CacheControl,
        Connection,
        ContentDisposition,
        ContentEncoding,
        ContentLanguage,
        ContentLength,
        ContentLocation,
        ContentMD5,
        ContentRange,
        ContentSecurityPolicy,
        ContentType,
        Cookie,
        DNT,
        Date,
        DeltaBase,
        ETag,
        Expect,
        Expires,
        Forwarded,
        From,
        FrontEndHttps,
        HTTP2Settings,
        Host,
        IM,
        IfMatch,
        IfModifiedSince,
        IfNoneMatch,
        IfRange,
        IfUnmodifiedSince,
        LastModified,
        Link,
        Location,
        MaxForwards,
        Origin,
        P3P,
        Pragma,
        ProxyAuthenticate,
        ProxyAuthorization,
        ProxyConnection,
        PublicKeyPins,
        Range,
        Referer,
        Refresh,
        RetryAfter,
        SaveData,
        SecWebSocketAccept,
        SecWebSocketKey,
        SecWebSocketVersion,
        Server,
        SetCookie,
        Status,
        StrictTransportSecurity,
        TE,
        TimingAllowOrigin,
        Tk,
        Trailer,
        TransferEncoding,
        Upgrade,
        UpgradeInsecureRequests,
        UserAgent,
        Vary,
        Via,
        WWWAuthenticate,
        Warning,
        XATTDeviceId,
        XContentDuration,
        XContentSecurityPolicy,
        XContentTypeOptions,
        XCorrelationID,
        XCsrfToken,
        XForwardedFor,
        XForwardedHost,
        XForwardedProto,
        XFrameOptions,
        XHttpMethodOverride,
        XPoweredBy,
        XRequestID,
        XRequestedWith,
        XUACompatible,
        XUIDH,
        XWapProfile,
        XWebKitCSP,
        XXSSProtection,
    };

    HttpHeader(Code code);
    HttpHeader(std::string const & name);
    HttpHeader(HttpHeader const & other) : code_(other.code_), name_(other.name_)   {                                                           }

    HttpHeader &        operator = (HttpHeader const & other)                       { code_ = other.code_; name_ = other.name_; return *this;   }
    friend bool         operator == (HttpHeader const & lhs, HttpHeader const & rhs);
    size_t              getHash() const noexcept;
    std::string const & getFieldName() const;

private:
    Code        code_;
    std::string name_;

    class Mapping {
    public:
        Mapping();

        std::unordered_map<Code, std::string, std::hash<int>> mapByCode_;
        std::unordered_map<std::string, Code>                 mapByName_;
    };

    Mapping & getMap() const;
};

typedef std::unordered_map<HttpHeader, std::string> HttpHeaderMap;

//--------------------------------------------------------------
// Specialize std::hash to support STL containers.
//--------------------------------------------------------------

namespace std {
    template<> struct hash<HttpHeader> {
        std::size_t operator()(HttpHeader const & s) const noexcept {
            return s.getHash();
        }
    };
}

//--------------------------------------------------------------

#endif

//========================================================================
