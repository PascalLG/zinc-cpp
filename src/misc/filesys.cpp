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

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/stat.h> 
#include <dirent.h>
#include <unistd.h>
#endif
#include <cstring>
#include <cerrno>
#include <algorithm>

#include "string.h"
#include "filesys.h"

//========================================================================
// fs::dirent
//
// Represent a directory entry. The fs::filepath::getDirectoryContent
// method returns objects of this type.
//========================================================================

#ifdef _WIN32
char const fs::pathSeparator = '\\';
#else
char const fs::pathSeparator = '/';
#endif

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

fs::dirent::dirent(char const * name, bool isdir, unsigned long size, date const & mtime)
    : name_(name),
      isdir_(isdir),
      size_(isdir ? 0 : size),
      mtime_(mtime) {
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
    size_t first = 0;
#ifdef _WIN32
    if (path_.size() >= 2 && isalpha(path_[0]) && path_[1] == ':') {
        first = 2;
    }
#endif
    size_t s = path_.rfind(pathSeparator);
    if (s != std::string::npos) {
        if (s > first) {
            return filepath(path_.substr(0, s));
        }
    } else if (!first) {
        return filepath(std::string("."));
    }
    std::string dir = path_.substr(0, first);
    dir.push_back(pathSeparator);
    return filepath(dir);
}

//--------------------------------------------------------------
// Access the filesystem to determine the kind of object this
// file path points to. Return one of the fs::type enumeration.
//--------------------------------------------------------------

fs::type fs::filepath::getFileType() const {
#ifdef _WIN32
    std::wstring s = UTF8ToWideString(path_);
    DWORD attr = GetFileAttributesW(s.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) {
        switch (GetLastError()) {
        case ERROR_FILE_NOT_FOUND:  return errorNotFound;
        case ERROR_PATH_NOT_FOUND:  return errorNotFound;
        case ERROR_ACCESS_DENIED:   return errorPermission;
        default:                    return errorOther;
        }
    } else if (attr & FILE_ATTRIBUTE_DIRECTORY) {
        return directory;
    } else {
        return file;
    }
#else
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
#endif
}

//--------------------------------------------------------------
// Enumerate the content of the directory this file path points
// to, calling the provided function once for each entry.
//--------------------------------------------------------------

void fs::filepath::getDirectoryContent(std::function<void(fs::dirent const &)> const & callback) const {
    std::string path = path_;
    if (path.back() != pathSeparator) {
        path.push_back(pathSeparator);
    }
#ifdef _WIN32
    path.push_back('*');
    WIN32_FIND_DATAW ffd;
    HANDLE hf = FindFirstFileW(UTF8ToWideString(path).c_str(), &ffd);
    if (hf != INVALID_HANDLE_VALUE) {
        do {
            if (ffd.cFileName[0] != '.') {
                ULARGE_INTEGER filesize;
                filesize.LowPart = ffd.nFileSizeLow;
                filesize.HighPart = ffd.nFileSizeHigh;
                dirent entry(WideStringToUTF8(ffd.cFileName).c_str(), (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0, static_cast<unsigned long>(filesize.QuadPart), date(FileTimeToPOSIX(ffd.ftLastWriteTime)));
                callback(entry);
            }
        } while (FindNextFileW(hf, &ffd));
        FindClose(hf);
    }
#else
    DIR * dp = opendir(path.c_str());
    if (dp) {
        struct ::dirent * ep;
        while ((ep = readdir(dp)) != nullptr) {
            if (ep->d_name[0] != '.') {
                std::string abspath = path + ep->d_name;
                struct stat st;
                if (!stat(abspath.c_str(), &st)) {
                    dirent entry(ep->d_name, S_ISDIR(st.st_mode), st.st_size, st.st_mtime);
                    callback(entry);
                }
            }
        }
        closedir(dp);
     }
#endif
}

//--------------------------------------------------------------
// Return the file last modification date. If we could not
// retrieve this information, return the current date/time as
// if the file was just modified.
//--------------------------------------------------------------

date fs::filepath::getModificationDate() const {
#ifdef _WIN32
    std::wstring s = UTF8ToWideString(path_);
    HANDLE h = CreateFileW(s.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (h != INVALID_HANDLE_VALUE) {
        FILETIME ft;
        BOOL ok = GetFileTime(h, nullptr, nullptr, &ft);
        CloseHandle(h);
        if (ok) {
            return date(FileTimeToPOSIX(ft));
        }
    }
#else
    struct stat st;
    if (stat(path_.c_str(), &st) >= 0) {
        return date(st.st_mtime);
    }
#endif
    return date::now();
}

//--------------------------------------------------------------
// Indicate whether a path is absolute or relative.
//--------------------------------------------------------------

bool fs::filepath::isAbsolute() const {
#ifdef _WIN32
    return path_.size() >= 3 && isalpha(path_[0]) && path_[1] == ':' && path_[2] == pathSeparator;
#else
    return !path_.empty() && path_.front() == pathSeparator;
#endif
}

//--------------------------------------------------------------
// Make an absolute filepath from a relative one. Return the path
// unchanged if it is already absolute.
//--------------------------------------------------------------

fs::filepath fs::filepath::makeAbsolute() const {
#ifdef _WIN32
    std::wstring path = UTF8ToWideString(path_);
    WCHAR * buffer = _wfullpath(nullptr, path.c_str(), 0);
    filepath res(WideStringToUTF8(buffer));
    free(buffer);
#else
    char * buffer = realpath(path_.c_str(), nullptr);
    filepath res(buffer);
    free(buffer);
#endif
    return res;
}

//--------------------------------------------------------------
// Append a component to a file path, handling path separators.
//--------------------------------------------------------------

fs::filepath & fs::filepath::operator += (fs::filepath const & component) {
    if (component.isAbsolute() || (!component.path_.empty() && component.path_.front() == pathSeparator)) {
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
// Helper file functions.
//========================================================================

//--------------------------------------------------------------
// Build a relative local filepath from a URI.
//--------------------------------------------------------------

fs::filepath fs::makeFilepathFromURI(std::string const & uri) {
    std::string ret(".");
    if (uri.empty() || uri.front() != '/') {
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
#ifdef _WIN32
    WCHAR path[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, path);
    return filepath(WideStringToUTF8(path));
#else
    char * buffer = getcwd(nullptr, 0);
    filepath res(buffer);
    free(buffer);
    return res;
#endif
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

void fs::enumSystemPaths(std::function<bool(fs::filepath const &)> callback) {
#ifdef _WIN32
    DWORD size = GetEnvironmentVariableW(L"PATH", nullptr, 0);
    if (size > 0) {
        std::wstring paths(size, 0);
        GetEnvironmentVariableW(L"PATH", &paths.front(), size);
        string::split(WideStringToUTF8(paths.c_str()), ';', 0, string::trim_both, [&callback] (std::string & path) {
            fs::filepath fp(path);
            return callback(fp);
        });
    }
#else
    std::string paths = getenv("PATH");
    string::split(paths, ':', 0, string::trim_none, [&callback] (std::string & path) {
        fs::filepath fp(path);
        return callback(fp);
    });
#endif
}

//--------------------------------------------------------------
// Return true if both stdout and stderr are a terminal.
//--------------------------------------------------------------

bool fs::isTTY() {
#ifdef _WIN32
	DWORD mode;	// hack: if retrieving the console mode fails, then outputs are probably redirected
    bool s1 = GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &mode) == 0 && GetLastError() == ERROR_INVALID_HANDLE;
	bool s2 = GetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), &mode) == 0 && GetLastError() == ERROR_INVALID_HANDLE;
	return !s1 || !s2;
#else
    return isatty(1) && isatty(2);
#endif
}

//========================================================================
