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

#include <sys/types.h> 
#include <sys/stat.h> 
#include <dirent.h>
#include <unistd.h>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <algorithm>

#include "misc.h"
#include "filesys.h"

char const fs::pathSeparator = '/';

//========================================================================
// fs::dirent
//
// Represent a directory entry. The fs::filepath::getDirectoryContent
// method returns objects of this type.
//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

fs::dirent::dirent(char const * name, bool isdir, unsigned long size, date mtime)
    : name_(name),
      isdir_(isdir),
      size_(isdir ? 0 : size),
      mtime_(mtime) {
}

//--------------------------------------------------------------
// Copy constructor.
//--------------------------------------------------------------

fs::dirent::dirent(dirent const & other)
    : name_(other.name_),
      isdir_(other.isdir_),
      size_(other.size_),
      mtime_(other.mtime_) {
}

//--------------------------------------------------------------
// Set of functions to compare entries by name, date or size.
//--------------------------------------------------------------

namespace fs {

    int compareByName(dirent const & lhs, dirent const & rhs) {
        int r = rhs.isdir_ - lhs.isdir_;
        if (r == 0) {
            r = lhs.name_.compare(rhs.name_);
        }
        return r;
    }

    int compareByDate(dirent const & lhs, dirent const & rhs) {
        int r = rhs.isdir_ - lhs.isdir_;
        if (r == 0) {
            r = lhs.mtime_.compare(rhs.mtime_);
            if (r == 0) {
                r = lhs.name_.compare(rhs.name_);
            }
        }
        return r;
    }

    int compareBySize(dirent const & lhs, dirent const & rhs) {
        int r = rhs.isdir_ - lhs.isdir_;
        if (r == 0) {
            r = lhs.size_ < rhs.size_ ? -1 : (lhs.size_ > rhs.size_ ? 1 : 0);
            if (r == 0) {
                r = lhs.name_.compare(rhs.name_);
            }
        }
        return r;
    }
}

//========================================================================
// fs::filepath
//
// Represent a generic filepath, which can be absolute or relative, and
// point to either a file or a directory. Basically, this class is a
// std::string with specific behaviors to handle path separators, file
// extensions, and so on.
//
// Using a specific type for filepaths provides:
// - type safety (you cannot pass a local path where a URI is
//   expected, for example)
// - portability by encapsulating and hiding plateform specific
//   details.
//========================================================================

//--------------------------------------------------------------
// Extract the extension of a filename. Return an empty string
// if the file has no extension.
//--------------------------------------------------------------

std::string fs::filepath::getExtension() const {
    size_t s = path_.rfind(pathSeparator);
    size_t e = path_.rfind('.');
    return (e != std::string::npos && (e > s || s == std::string::npos)) ? path_.substr(e) : std::string();
}

//--------------------------------------------------------------
// Return the last component of a file path. (Or the file path
// itself if it has only one component.)
//--------------------------------------------------------------

fs::filepath fs::filepath::getLastComponent() const {
    size_t s = path_.rfind(pathSeparator);
    return (s != std::string::npos) ? path_.substr(s + 1) : path_;
}

//--------------------------------------------------------------
// Return the directory, i.e. the filepath without the filename.
//--------------------------------------------------------------

fs::filepath fs::filepath::getDirectory() const {
    size_t s = path_.rfind(pathSeparator);
    return filepath((s != std::string::npos) ? ((s > 0) ? path_.substr(0, s) : std::string(1, pathSeparator)) : ".");
}

//--------------------------------------------------------------
// Access the filesystem to determine the kind of object this
// file path points to. Return one of the fs::type enumeration.
//--------------------------------------------------------------

fs::type fs::filepath::getFileType() const {
    struct stat st;
    if (!stat(path_.c_str(), &st)) {
        if (S_ISREG(st.st_mode)) {
            return file;
        } else if (S_ISDIR(st.st_mode)) {
            return directory;
        } else {
            return errorNotFound;
        }
    } else {
        switch (errno) {
            case ENOENT:    return errorNotFound;
            case ENOTDIR:   return errorNotFound;
            case EACCES:    return errorPermission;
            default:        return errorOther;
        }
    }
}

//--------------------------------------------------------------
// Enumerate the content of the directory this file path points
// to, calling the provided function once for each entry.
//--------------------------------------------------------------

void fs::filepath::getDirectoryContent(std::function<void(fs::dirent const &)> const & callback) const {
#ifdef _WIN32

#else
    DIR * dp = opendir(path_.c_str());
    if (dp) {
        struct ::dirent * ep;
        while ((ep = readdir(dp)) != nullptr) {
            struct stat st;
            if (!stat(ep->d_name, &st)) {
                dirent entry(ep->d_name, S_ISDIR(st.st_mode), st.st_size, st.st_mtime);
                callback(entry);
            }
        }
        closedir(dp);
     }
#endif
}

//--------------------------------------------------------------
// Return the file last modification date. In we could not
// retrieve this information, return the current date/time as
// if the file was just modified.
//--------------------------------------------------------------

date fs::filepath::getModificationDate() const {
    struct stat st;
    if (stat(path_.c_str(), &st) < 0) {
        return date::now();
    } else {
        return date(st.st_mtime);
    }
}

//--------------------------------------------------------------
// Indicate whether a path is absolute or relative.
//--------------------------------------------------------------

bool fs::filepath::isAbsolute() const {
    return !path_.empty() && path_.front() == pathSeparator;
}

//--------------------------------------------------------------
// Make an absolute filepath from a relative one. Return the path
// unchanged if it is already absolute.
//--------------------------------------------------------------

fs::filepath fs::filepath::makeAbsolute() const {
    char * buffer = realpath(path_.c_str(), nullptr);
    filepath res(buffer);
    free(buffer);
    return res;
}

//--------------------------------------------------------------
// Append a component to a file path, handling path separators.
//--------------------------------------------------------------

fs::filepath & fs::filepath::operator += (fs::filepath const & component) {
    if (component.isAbsolute()) {
        path_ = component.path_;
    } else {
        auto skipDotDir = [] (std::string const & s) {
            size_t pos = 0;
            int state = 0;
            while (pos < s.size()) {
                char ch = s[pos];
                switch (state) {
                case 0:
                    if (ch == '.') {
                        state = 1;
                    } else {
                        return pos;
                    }
                    break;
                case 1:
                    if (ch == pathSeparator) {
                        state = 2;
                    } else {
                        return pos - 1;
                    }
                    break;
                case 2:
                    if (ch == '.') {
                        state = 1;
                    } else if (ch != pathSeparator) {
                        return pos;
                    }
                    break;
                }
                ++pos;
            }
            return std::string::npos;
        };

        size_t pos = skipDotDir(component.path_);
        if (pos != std::string::npos) {
            if (path_.empty()) {
                path_.push_back('.');
            }
            if (path_.back() != pathSeparator) {
                path_.push_back(pathSeparator);
            }
            path_.append(component.path_, pos, std::string::npos);
        }
    }
    return *this;
}

//========================================================================
// fs::tmpfile
//
// Encapsulate a temporary file. The file is actually craeted the first
// time it is written to, and it is deleted when the last descriptor/
// handle to the file is closed.
//
// The implementation MUST use the plateform specific API and not rely on
// a higher level library (such as FILE* or std::fstream) because we
// need the raw file descriptor/handle to communicate with external
// processes.
//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

fs::tmpfile::tmpfile() 
  : fd_(-1) {
}

//--------------------------------------------------------------
// Destructor.
//--------------------------------------------------------------

fs::tmpfile::~tmpfile() {
    if (fd_ >= 0) {
        close(fd_);
    }
}

//--------------------------------------------------------------
// Write data to the file, creating and opening it if this is
// not already done.
//--------------------------------------------------------------

bool fs::tmpfile::write(void const * data, size_t length) {
    if (fd_ < 0) {
        char fname[32] = "tmp_XXXXXX";
        fd_ = mkstemp(fname);
        if (fd_ < 0) {
            return false;
        }
        unlink(fname);
    }
    return ::write(fd_, data, length) == static_cast<ssize_t>(length);
}

//--------------------------------------------------------------
// Return the size of the file.
//--------------------------------------------------------------

size_t fs::tmpfile::getSize() const {
    size_t result;
    if (fd_ >= 0) {
        off_t previous = lseek(fd_, 0, SEEK_CUR);
        result = lseek(fd_, 0, SEEK_END);
        lseek(fd_, previous, SEEK_SET);
    } else {
        result = 0;
    }
    return result;
}

//========================================================================
// Helper file functions.
//========================================================================

//--------------------------------------------------------------
// Build a relative local filepath from a URI.
//--------------------------------------------------------------

fs::filepath fs::makeFilepathFromURI(std::string const & uri) {
    std::string ret(".");
    if (uri.front() != '/') {
        ret.push_back('/');
    }
    ret += uri;

    if (pathSeparator != '/') {
        std::transform(ret.begin(), ret.end(), ret.begin(), [] (char ch) {
            return ch == '/' ? pathSeparator : ch;
        });
    }

    return ret;
}

//--------------------------------------------------------------
// Return the current working directory.
//--------------------------------------------------------------

fs::filepath fs::getCurrentDirectory() {
    char * buffer = getcwd(nullptr, 0);
    filepath res(buffer);
    free(buffer);
    return res;
}

//--------------------------------------------------------------
// Return the hostname.
//--------------------------------------------------------------

std::string fs::getHostName() {
    char buffer[256];
    if (gethostname(buffer, sizeof(buffer) - 1) < 0) {
        strcpy(buffer, "localhost");
    }
    return std::string(buffer);
}

//--------------------------------------------------------------
// Enumerate the directories in the system PATH environement
// variable, by calling a function once for each entry.
//--------------------------------------------------------------

void fs::enumSystemPaths(std::function<bool(fs::filepath const &)> const & callback) {
    std::string paths = getenv("PATH");
    string::split(paths, ':', 0, string::trim_none, [&callback] (std::string & path) {
        fs::filepath fp(path);
        return callback(fp);
    });
}

//--------------------------------------------------------------
// Return true if both stdout and stderr are a terminal.
//--------------------------------------------------------------

bool fs::isTTY() {
    return isatty(1) && isatty(2);
}

//========================================================================
