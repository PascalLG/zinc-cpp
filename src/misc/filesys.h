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

#ifndef FILESYS_H
#define FILESYS_H

#include <string>
#include <functional>
#include <ostream>
#include <vector>

#include "portability.h"
#include "date.h"

//--------------------------------------------------------------
// Portable file system functions.
//--------------------------------------------------------------

namespace fs {

    extern char const pathSeparator;

    enum type {
        errorNotFound,
        errorPermission,
        errorOther,
        file,
        directory,
    };

    class dirent {
    public:
        dirent(char const * name, bool isdir, unsigned long size, date const & mtime);
        dirent(dirent const & other) = default;

        std::string const & getName() const                                                     { return name_;                         }
        type                getFileType() const                                                 { return isdir_ ? directory : file;     }
        unsigned long       getSize() const                                                     { return size_;                         }
        date                getModificationDate() const                                         { return mtime_;                        }

        friend int          compareByName(dirent const & lhs, dirent const & rhs);
        friend int          compareByDate(dirent const & lhs, dirent const & rhs);
        friend int          compareBySize(dirent const & lhs, dirent const & rhs);

    private:
        std::string         name_;      // directory entry name
        bool                isdir_;     // is the entry a directory?
        unsigned long       size_;      // size (if not a directory)
        date                mtime_;     // date/time of last modification
    };

    class filepath {
    public:
        filepath(char const * path) : path_(path)                                               {                                       }
        filepath(std::string const & path) : path_(path)                                        {                                       }
        filepath(filepath const & other) : path_(other.path_)                                   {                                       }

        filepath &              operator = (filepath const & other)                             { path_ = other.path_; return *this;    }

        std::string             getExtension() const;
        filepath                getLastComponent() const;
        filepath                getDirectory() const;
        type                    getFileType() const;
        void                    getDirectoryContent(std::function<void(dirent const &)> const & callback) const;
        date                    getModificationDate() const;

        bool                    isAbsolute() const;
        filepath                makeAbsolute() const;
        filepath &              operator += (filepath const & component);

        std::string const &     getStdString() const                                            { return path_;                         }
        char const *            getCString() const                                              { return path_.c_str();                 }

        friend filepath         operator + (filepath const & lhs, filepath const & rhs)         { filepath t(lhs); return t += rhs;     }
        friend bool             operator == (filepath const & lhs, filepath const & rhs)        { return lhs.path_ == rhs.path_;        }
        friend std::ostream &   operator << (std::ostream & os, filepath const & rhs)           { return os << rhs.path_;               }

    private:
        std::string             path_;
    };

    filepath        makeFilepathFromURI(std::string const & uri);
    filepath        getCurrentDirectory();
    std::string     getHostName();
    void            enumSystemPaths(std::function<bool(filepath const &)> callback);
    bool            isTTY();
}

//--------------------------------------------------------------
// Specialize std::hash for fs::filepath.
//--------------------------------------------------------------

namespace std {
    template <> struct hash<fs::filepath> {
        size_t operator() (fs::filepath const & x) const noexcept {
            return hash<string>()(x.getStdString());
        }
    };
}

//--------------------------------------------------------------

#endif

//========================================================================
