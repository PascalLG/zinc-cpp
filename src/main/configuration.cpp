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

#include <cstring>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "../misc/string.h"
#include "configuration.h"

//========================================================================
// Variant
//
// Container to encapsulate parameter values. Support boolean, integer
// and text values.
//========================================================================

//--------------------------------------------------------------
// Constructors for boolean, integer and text values.
//--------------------------------------------------------------

Variant::Variant(bool v)
    : type_(boolean), 
      vbool_(v),
      vint_(0),
      vstring_() {
}

Variant::Variant(int v)
    : type_(integer), 
      vbool_(false),
      vint_(v),
      vstring_() {
}

Variant::Variant(char const * v)
    : type_(string),
      vbool_(false),
      vint_(0),
      vstring_(v) {
}

Variant::Variant(std::string const & v)
    : type_(string),
      vbool_(false),
      vint_(0),
      vstring_(v) {
}

//--------------------------------------------------------------
// Print the value to a string.
//--------------------------------------------------------------

std::string Variant::print() const {
    std::string result;

    switch (type_) {
    case boolean:   result = vbool_ ? "yes" : "no";     break;
    case integer:   result = std::to_string(vint_);     break;
    case string:    result = vstring_;                  break;
    default:                                            break;
    }

    return result;
}

//--------------------------------------------------------------
// Parse a value from a string. Return true if parsing succeeds,
// false if an error occurs.
//--------------------------------------------------------------

bool Variant::parse(std::string const & str) {
    switch (type_) {
    case boolean:
        if (string::compare_i(str, "yes") || string::compare_i(str, "true")) {
            vbool_ = true;
        } else if (string::compare_i(str, "no") || string::compare_i(str, "false")) {
            vbool_ = false;
        } else {
            return false;
        }
        break;
    case integer:
        vint_ = string::to_long(str, 10);
        if (vint_ < 0) {
            return false;
        }
        break;
    case string:
        vstring_ = str;
        break;
    }
    return true;
}

//========================================================================
// Configuration
//
// Store all the configuration parameters for the server. Parameters
// are grouped in blocks. The first block is named "Server" and store
// general parameters; then we have one block for each script language
// we support.
//========================================================================

//--------------------------------------------------------------
// Constructor. Initialize all configuration parameters to their
// default values.
//--------------------------------------------------------------

Configuration::Configuration()
    : general_("Server"),
      cgis_(),
      extensions_() {

    // Initialize the server parameter block with default
    // values.

    static char const * indexes = "index.html index.xhtml index.htm index.php index.py";
    std::string host = fs::getHostName();

    general_.setContent({
        { optListen,                8080,                   [] (Variant & x) { return x.getIntegerValue() >= 1024 && x.getIntegerValue() <= 65535; }    },
        { optLimitThreads,          8,                      [] (Variant & x) { return x.getIntegerValue() > 0 && x.getIntegerValue() < 32; }            },
        { optLimitRequestLine,      2048,                   [] (Variant & x) { return x.getIntegerValue() >= 256 && x.getIntegerValue() <= 655535; }    },
        { optLimitRequestHeaders,   8192,                   [] (Variant & x) { return x.getIntegerValue() >= 256 && x.getIntegerValue() <= 655535; }    },
        { optLimitRequestBody,      32 * 1024 * 1024,       [] (Variant & x) { return x.getIntegerValue() > 0; }                                        },
        { optCompression,           true,                   nullptr                                                                                     },
        { optDirectoryIndex,        indexes,                nullptr                                                                                     },
        { optDirectoryListing,      true,                   nullptr                                                                                     },
        { optTimeout,               30,                     [] (Variant & x) { return x.getIntegerValue() > 0 && x.getIntegerValue() < 600; }           },
        { optExpires,               3600,                   [] (Variant & x) { return x.getIntegerValue() >= 0; }                                       },
        { optServerAdmin,           "admin@" + host,        nullptr                                                                                     },
        { optServerName,            host,                   nullptr                                                                                     },
    });

    // Initialize the parameter blocks for PHP and Python
    // scripts with default values, and rebuild the extension
    // map.

    cgis_.emplace_back("PHP",    "php php7", "php-cgi",  "" );
    cgis_.emplace_back("Python", "py",       "python",   "" );
    buildExtentionMap();
}

//--------------------------------------------------------------
// Build the extension map. This map is used to determine if
// a given file is a script and which interpreter to spawn.
//--------------------------------------------------------------

void Configuration::buildExtentionMap() {
    extensions_.clear();
    for (CGI & cgi: cgis_) {
        if (!cgi.at(optInterpreter).getStringValue().empty()) {
            string::split(cgi.at(optExtensions).getStringValue(), ' ', 0, string::trim_both, [this, &cgi] (std::string & ext) {
                string::lowercase(ext);
                this->extensions_.emplace(ext, cgi);
                return true;
            });
        }
    }
}

//--------------------------------------------------------------
// Dump all the configuration parameters to stdout, one line
// for each key/value pair, according to the following format:
//
// section.key1 = value1
// section.key2 = value2
// ...
//
// This format is used to print the server configuration at
// startup.
//--------------------------------------------------------------

void Configuration::log() {
    general_.print(std::cout, false);
    for (CGI & cgi: cgis_) {
        cgi.print(std::cout, false);
    }
    std::cout << std::endl;
}

//--------------------------------------------------------------
// Dump all the configuration parameters to a file, according
// to the following format:
//
// [section1]
// key1 = value1
// key2 = value2
// ...
// [section2]
// key3 = value3
// key4 = value4
// ...
//
// This format is used for the zinc.ini configuration file.
//--------------------------------------------------------------

bool Configuration::save(fs::filepath const & filename) {
    std::ofstream fs(filename.getCString(), std::ios::trunc);
    return fs.good() && save(fs);
}

bool Configuration::save(std::ostream & fs) {
    fs << "###############################" << std::endl;
    fs << "#   Zinc configuration file   #" << std::endl;
    fs << "###############################" << std::endl;
    fs << std::endl;
    general_.print(fs, true);
    fs << std::endl;
    for (CGI & cgi: cgis_) {
        cgi.print(fs, true);
        fs << std::endl;
    }
    return true;
}

//--------------------------------------------------------------
// Load configuration parameters from a file. The accepted file
// format is the same as generated by the save() method above.
// If the file is missing or cannot be read, it is simply ignored.
// If a parameter is not defined in the file, it keeps the value
// it has before this method is called.
//
// Return false if an error occurs. A missing parameter file
// is not considered as an error, but a syntax error in the file
// itself is.
//--------------------------------------------------------------

bool Configuration::load(fs::filepath const & filename) {
    std::ifstream fs(filename.getStdString(), std::ifstream::in);
    return !fs.good() || load(fs, filename);
}

bool Configuration::load(std::istream & fs, fs::filepath const & filename) {
    ParameterBlock * block = nullptr;
    int linecount = 0;
    bool ok = true;

    std::string line;
    while (std::getline(fs, line)) {
        linecount++;

        // Ignore comments and leading/trailing spaces.

        size_t cpos = line.find('#');
        if (cpos != std::string::npos) {
            line.erase(cpos);
        }
        string::trim(line, string::trim_both);

        if (!line.empty()) {
            if (line.front() == '[' && line.back() == ']') {

                // Section definition. Retrieve the corresponding
                // paramater block, create it if necessary.

                std::string section = line.substr(1, line.size() - 2);
                if (section == "Server") {
                    block = &general_;
                } else {
                    auto got = std::find_if(cgis_.begin(), cgis_.end(), [&] (CGI const & c) { return c.getSectionName() == section; });
                    if (got == cgis_.end()) {
                        cgis_.emplace_back(section.c_str(), "", "", "");
                        block = &cgis_.back();
                    } else {
                        block = &*got;
                    }
                }
            } else {

                // Key/value pair. Parse the value and insert it in
                // the current block.

                size_t epos = line.find('=');
                if (epos == std::string::npos) {
                    std::cerr << "error: " << filename <<"(" << linecount << "): expecting '<parameter> = <value>' pair" << std::endl;
                    ok = false;
                } else {
                    std::string key = line.substr(0, epos);
                    string::trim(key, string::trim_right);
                    std::string value = line.substr(epos + 1);
                    string::trim(value, string::trim_left);
                    if (!block) {
                        std::cerr << "error: " << filename <<"(" << linecount << "): parameter defined outside a section" << std::endl;
                        ok = false;
                    } else {
                        std::string err;
                        if (!block->loadParameter(key, value, err)) {
                            std::cerr << "error: " << filename <<"(" << linecount << "): " << err << std::endl;
                            ok = false;
                        }
                    }
                }
            }
        }
    }

    buildExtentionMap();
    return ok;
}

//--------------------------------------------------------------
// Return the CGI interpreter to spawn to execute a script file,
// based on the extension of its filename. Return NULL if the
// extension is not registered.
//--------------------------------------------------------------

Configuration::CGI const * Configuration::getInterpreter(fs::filepath const & filename) const {
    std::string ext = filename.getExtension();
    if (ext.length() > 1 && ext.front() == '.') {
        ext.erase(0, 1);
        string::lowercase(ext);
        auto got = extensions_.find(ext);
        if (got != extensions_.end()) {
            return &got->second;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------
// Return the list of resources to look for when the client
// requests the content of a directory.
//--------------------------------------------------------------

std::vector<std::string> Configuration::getDirectoryIndexes() const {
    std::vector<std::string> result;
    string::split(general_.at(optDirectoryIndex).getStringValue(), ' ', 0, string::trim_none, [&result] (std::string & name) {
        result.push_back(name);
        return true;
    });
    return result;
}

//--------------------------------------------------------------
// Keys for the various option name. WARNING: for the sake of
// efficiency, maps that store configuration parameters are
// indexed by string ADDRESSES, not by string CONTENT.
//--------------------------------------------------------------

char const * Configuration::optListen               = "Listen";
char const * Configuration::optLimitThreads         = "LimitThreads";
char const * Configuration::optLimitRequestLine     = "LimitRequestLine";
char const * Configuration::optLimitRequestHeaders  = "LimitRequestHeaders";
char const * Configuration::optLimitRequestBody     = "LimitRequestBody";
char const * Configuration::optCompression          = "Compression";
char const * Configuration::optDirectoryIndex       = "DirectoryIndex";
char const * Configuration::optDirectoryListing     = "DirectoryListing";
char const * Configuration::optTimeout              = "Timeout";
char const * Configuration::optExpires              = "Expires";
char const * Configuration::optServerAdmin          = "ServerAdmin";
char const * Configuration::optServerName           = "ServerName";
char const * Configuration::optExtensions           = "Extensions";
char const * Configuration::optInterpreter          = "Interpreter";
char const * Configuration::optCmdLine              = "CmdLine";

//========================================================================
// Configuration::ParameterBlock
//
// Group a list of related parameters together. Such a block has a name
// and store a list of key/value pairs.
//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

Configuration::ParameterBlock::ParameterBlock(char const * section) 
    : section_(section) {
}

//--------------------------------------------------------------
// Set the content of the block, as a list of parameters with
// their initial value.
//--------------------------------------------------------------

void Configuration::ParameterBlock::setContent(std::initializer_list<Configuration::ParameterBlock::Parameter> const & content) {
    for (Parameter const & e: content) {
        keys_.emplace_back(e.name);
        values_.emplace(e.name, e.value);
        if (e.validator) {
            validators_.emplace(e.name, e.validator);
        }
    }
}

//--------------------------------------------------------------
// Load the value of a parameter from a string. Return
//--------------------------------------------------------------

bool Configuration::ParameterBlock::loadParameter(std::string const & name, std::string const & value, std::string & err) {
    auto got = std::find_if(values_.begin(), values_.end(), [&] (std::pair<char const *, Variant> const & x) { return name == x.first; });
    if (got == values_.end()) {
        err = "unknown parameter: " + name;
        return false;
    } else {
        auto got2 = validators_.find(got->first);
        bool (* validator)(Variant &) = got2 != validators_.end() ? got2->second : nullptr;
        Variant & v = got->second;
        if (!v.parse(value) || (validator && !validator(v))) {
            err = "invalid value for parameter: " + name;
            return false;
        }
    }
    return true;
}

//--------------------------------------------------------------
// Dump all the parameters of this block.
//--------------------------------------------------------------

void Configuration::ParameterBlock::print(std::ostream & out, bool file) {
    if (file) {
        out << "[" << section_ << "]" << std::endl;
    }
    for (auto & e: keys_) {
        if (!file) {
            out << "    " << section_ << ".";
        }
        out << e << " = " << values_.at(e).print() << std::endl;
    }
}

//========================================================================
// Configuration::CGI
//
// Specialization of a parameter block for a CGI script. Contains
// parameters that are specific to an interpreter.
//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

Configuration::CGI::CGI(char const * section, char const * extensions, char const * interpreter, char const * cmdline)
    : ParameterBlock(section) {

    // Try to locate the interpreter, if the filename is not
    // already absolute.

    std::string exe;
    if (strchr(interpreter, fs::pathSeparator) == nullptr) {
        fs::enumSystemPaths([&exe, interpreter] (fs::filepath const & path) {
            fs::filepath tmp = path + interpreter;
            if (tmp.getFileType() == fs::file) {
                exe = tmp.getStdString();
                return false;
            }
            return true;
        });
    } else {
        exe = interpreter;
    }

    // Initialize the parameter block.

    setContent({
        { optExtensions,  extensions,   nullptr },
        { optInterpreter, exe,          nullptr },
        { optCmdLine,     cmdline,      nullptr },
    });
}

//========================================================================
