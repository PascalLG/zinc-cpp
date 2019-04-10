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

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <cassert>
#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <unordered_map>

#include "version.h"
#include "filesys.h"

//--------------------------------------------------------------
// Container to encapsulate parameter values.
//--------------------------------------------------------------

class Variant {
public:
    Variant(bool v);
    Variant(int v);
    Variant(char const * v);
    Variant(std::string const & v);

    std::string         print() const;
    bool                parse(std::string const & str);

    bool                getBooleanValue() const    { assert(type_ == boolean); return vbool_;   }
    int                 getIntegerValue() const    { assert(type_ == integer); return vint_;    }
    std::string const & getStringValue() const     { assert(type_ == string);  return vstring_; }

private:
    enum Type {
        boolean,
        integer,
        string,
    };

    Type        type_;      // Parameter type
    bool        vbool_;     // Storage for a parameter of type bool
    int         vint_;      // Storage for a parameter of type int
    std::string vstring_;   // Storage for a parameter of type string
};

//--------------------------------------------------------------
// Singleton object storing the application configuration.
//--------------------------------------------------------------

class Configuration {
private:
    void buildExtentionMap();

    class ParameterBlock {
    public:
        ParameterBlock(char const * section);

        struct Parameter {
            char const * name;
            Variant      value;
            bool      (* validator)(Variant &);
        };

        void    setContent(std::initializer_list<Parameter> const & content);
        bool    loadParameter(std::string const & name, std::string const & value, std::string & err);
        void    print(std::ostream & out, bool file);

        std::string const & getSectionName() const              { return section_;                                              }
        Variant &           at(char const * opt)                { return values_.at(opt);                                       }
        Variant const &     at(char const * opt) const          { return values_.at(opt);                                       }

    private:
        std::string                                             section_;       // Name of this block of parameters
        std::vector<char const *>                               keys_;          // List of parameters
        std::unordered_map<char const *, Variant>               values_;        // Value of each parameter
        std::unordered_map<char const *, bool (*)(Variant &)>   validators_;    // Validation functions (for parameters that require one)
    };

public:
    class CGI : public ParameterBlock {
    public:
        CGI(char const * section, char const * extensions, char const * interpreter, char const * cmdline);

        fs::filepath    getInterpreter() const                  { return at(optInterpreter).getStringValue();                   }
        std::string     getCmdLine() const                      { return at(optCmdLine).getStringValue();                       }
    };

    void                        log();
    bool                        save(fs::filepath const & filename);
    bool                        load(fs::filepath const & filename);
    CGI const *                 getInterpreter(fs::filepath const & filename) const;

    void                        setListeningPort(int port)      { general_.at(optListen) = port;                                }
    int                         getListeningPort() const        { return general_.at(optListen).getIntegerValue();              }
    int                         getLimitThreads() const         { return general_.at(optLimitThreads).getIntegerValue();        }
    int                         getLimitRequestLine() const     { return general_.at(optLimitRequestLine).getIntegerValue();    }
    int                         getLimitRequestHeaders() const  { return general_.at(optLimitRequestHeaders).getIntegerValue(); }
    int                         getLimitRequestBody() const     { return general_.at(optLimitRequestBody).getIntegerValue();    }
    bool                        isCompressionEnabled() const    { return general_.at(optCompression).getBooleanValue();         }
    std::vector<std::string>    getDirectoryIndexes() const;
    bool                        isListingEnabled() const        { return general_.at(optDirectoryListing).getBooleanValue();    }
    int                         getTimeout() const              { return general_.at(optTimeout).getIntegerValue();             }
    int                         getExpires() const              { return general_.at(optExpires).getIntegerValue();             }
    std::string const &         getServerAdmin() const          { return general_.at(optServerAdmin).getStringValue();          }
    std::string const &         getServerName() const           { return general_.at(optServerName).getStringValue();           }

    static Configuration & getInstance();

private:
    ParameterBlock                          general_;           // General parameter block
    std::list<CGI>                          cgis_;              // Parameter blocks for each supported script language
    std::unordered_map<std::string, CGI &>  extensions_;        // Map to efficiently retrieve a CGI script from a filename extension

    static char const * optListen;                              // TCP/IP port the server listens to
    static char const * optLimitThreads;                        // Maximum number of worker threads
    static char const * optLimitRequestLine;                    // Maximum number of worker threads
    static char const * optLimitRequestHeaders;                 // Maximum number of worker threads
    static char const * optLimitRequestBody;                    // Maximum number of worker threads
    static char const * optCompression;                         // Enable/disable HTTP compression
    static char const * optDirectoryIndex;                      // Index files to search for when browsing a directory
    static char const * optDirectoryListing;                    // Generate a listing of directory contents
    static char const * optTimeout;                             // Timeout
    static char const * optExpires;                             // Default value for the Expires header
    static char const * optServerAdmin;                         // Email address of the server admin
    static char const * optServerName;                          // Server name
    static char const * optExtensions;                          // List of extensions (comma separated) for a CGI script
    static char const * optInterpreter;                         // Path to the interpreter to execute a CGI script
    static char const * optCmdLine;                             // Extra options to pass to the interpreter

public_for_testing:
    Configuration();

    bool save(std::ostream & fs);
    bool load(std::istream & fs, fs::filepath const & filename);
};

//--------------------------------------------------------------

#endif

//========================================================================
