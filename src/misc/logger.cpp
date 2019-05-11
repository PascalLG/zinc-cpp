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
#include <iostream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <algorithm>

#include "filesys.h"
#include "logger.h"

//--------------------------------------------------------------
// Local variables.
//--------------------------------------------------------------

static std::mutex                                       loggerMutex;
static std::unordered_map<std::thread::id, std::string> threadList;
static logger::level                                    minLevel = logger::trace;
static bool                                             dumpBodies = false;
static bool                                             useAnsiSequences = true;

//========================================================================
// Support for ANSI sequences.
//========================================================================

//--------------------------------------------------------------
// Enable/disable ANSI sequences in output. Note: if stdout or
// stderr is not a terminal, sequences are always disabled.
//--------------------------------------------------------------

void ansi::setEnabled(bool colors) {
    useAnsiSequences = colors && fs::isTTY();
}

//--------------------------------------------------------------
// Return the ANSI sequence for a given color.
//--------------------------------------------------------------

std::string ansi::getSequence(color color) {
    if (useAnsiSequences) {
        if (color >= 0 && color <= 15) {
            char buffer[16];
            sprintf(buffer, "\x1B[38;5;%dm", color);
            return std::string(buffer);
        } else {
            return std::string("\x1B[39m\x1B[22m");
        }
    } else {
        return std::string();
    }
}

//--------------------------------------------------------------
// Set the log level.
//--------------------------------------------------------------

void logger::setLevel(level loglevel, bool logdump) {
    minLevel = loglevel;
    dumpBodies = logdump;
}

//--------------------------------------------------------------
// Register a user friendly-name for each worker thread.
//--------------------------------------------------------------

void logger::registerWorkerThread(int no) {
    std::unique_lock<std::mutex> lock(loggerMutex);
    threadList.emplace(std::this_thread::get_id(), no == 0 ? "main" : "#" + std::to_string(no));
}

//--------------------------------------------------------------
// Indicate if logging is enabled or not for a log of a given
// level.
//--------------------------------------------------------------

bool logger::isLogEnabled(level level) {
    return level >= minLevel;
}

//--------------------------------------------------------------
// Print a line of log. To avoid mixing messages of different
// threads, this function is protected by a mutex.
//--------------------------------------------------------------

void logger::print(char level, ansi::color color, std::ostringstream const & oss) {
    std::unique_lock<std::mutex> lock(loggerMutex);

    auto got = threadList.find(std::this_thread::get_id());
    std::cout << level << " "
              << date::now().format("%b %d %H:%M:%S", date::local) //std::put_time(&tm, "%b %d %H:%M:%S")
              << " ["
              << std::setw(4) << (got != threadList.end() ? got->second : "n/a") 
              << "] ";

    if (useAnsiSequences) {
        std::cout << ansi::getSequence(color);
    }
    std::cout << oss.str();
    if (useAnsiSequences) {
        std::cout << ansi::getSequence(ansi::def);
    }

    std::cout << std::endl;
}

//========================================================================
// logger::dump
//
// Helper class to dump request and response bodies as hexadecimal/ASCII
// content.
//========================================================================

//--------------------------------------------------------------
// Constructor.
//--------------------------------------------------------------

logger::dump::dump(ansi::color color, char const * prefix)
  : color_(color),
    prefix_(prefix),
    count_(0)  {
}

//--------------------------------------------------------------
// Destructor.
//--------------------------------------------------------------

logger::dump::~dump() {
    if (dumpBodies && count_ > 0) {
        emit(pending_, count_);
    }
}

//--------------------------------------------------------------
// Process a chunk of data, buffering as necessary.
//--------------------------------------------------------------

void logger::dump::write(void const * data, size_t length) {
    if (dumpBodies && length > 0) {
        uint8_t const * ptr = reinterpret_cast<uint8_t const *>(data);
        while (length > 0) {
            if (count_ > 0) {
                size_t rem = std::min(sizeof(pending_) - count_, length);
                memcpy(pending_ + count_, ptr, rem);
                count_ += rem;
                if (count_ >= sizeof(pending_)) {
                    emit(pending_, count_);
                    count_ = 0;
                }
                length -= rem;
                ptr += rem;
            } else if (length >= sizeof(pending_)) {
                emit(ptr, sizeof(pending_));
                length -= sizeof(pending_);
                ptr += sizeof(pending_);
            } else {
                memcpy(pending_, ptr, length);
                count_ = length;
                break;
            }
        }
    }
}

//--------------------------------------------------------------
// Emit a dump.
//--------------------------------------------------------------

void logger::dump::emit(uint8_t const * data, size_t length) {
    std::ostringstream oss;
    oss << prefix_ << ' ';

    oss.flags(std::ios::hex | std::ios::right);
    oss.fill('0');
    for (size_t i = 0; i < length; i++) {
        oss << std::setw(2) << static_cast<unsigned>(data[i]) << ' ';
    }
    for (size_t i = length; i < sizeof(pending_); i++) {
        oss << "   ";
    }

    oss << ' ';
    for (size_t i = 0; i < length; i++) {
        int ch = data[i];
        if (isprint(ch)) {
            oss << static_cast<char>(ch) << "  ";
        } else if (ch == '\n') {
            oss << "\\n ";
        } else if (ch == '\r') {
            oss << "\\r ";
        } else if (ch == '\t') {
            oss << "\\t ";
        } else {
            oss << "\xC2\xB7  "; // middle dot, UTF8 encoded
        }
    }

    print('T', color_, oss);
}

//========================================================================
