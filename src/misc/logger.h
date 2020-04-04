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

#ifndef LOGGER_H
#define LOGGER_H

#include <sstream>
#include <stdint.h>

//--------------------------------------------------------------
// LOG_* macros.
//--------------------------------------------------------------

#define LOG_TRACE(msg)          { if (logger::isLogEnabled(logger::trace)) { std::ostringstream oss; oss << msg; logger::print('T', ansi::lightGray,    oss); }}
#define LOG_DEBUG_SEND(msg)     { if (logger::isLogEnabled(logger::debug)) { std::ostringstream oss; oss << msg; logger::print('D', ansi::magenta,      oss); }}
#define LOG_DEBUG_RECV(msg)     { if (logger::isLogEnabled(logger::debug)) { std::ostringstream oss; oss << msg; logger::print('D', ansi::cyan,         oss); }}
#define LOG_INFO(msg)           { if (logger::isLogEnabled(logger::info))  { std::ostringstream oss; oss << msg; logger::print('I', ansi::yellow,       oss); }}
#define LOG_INFO_SEND(msg)      { if (logger::isLogEnabled(logger::info))  { std::ostringstream oss; oss << msg; logger::print('I', ansi::lightMagenta, oss); }}
#define LOG_INFO_RECV(msg)      { if (logger::isLogEnabled(logger::info))  { std::ostringstream oss; oss << msg; logger::print('I', ansi::lightCyan,    oss); }}
#define LOG_ERROR(msg)          { if (logger::isLogEnabled(logger::error)) { std::ostringstream oss; oss << msg; logger::print('E', ansi::lightRed,     oss); }}

//--------------------------------------------------------------
// Colour functions.
//--------------------------------------------------------------

namespace ansi {

    enum color : int {
        black,
        red,
        green,
        yellow,
        blue,
        magenta,
        cyan,
        lightGray,
        darkGray,
        lightRed,
        lightGreen,
        lightYellow,
        lightBlue,
        lightMagenta,
        lightCyan,
        white,
        def                 = 128,    // default terminal color
    };

    void        setupConsole();
    void        setEnabled(bool colors);
    std::string getSequence(color color);
}

//--------------------------------------------------------------
// Logger functions.
//--------------------------------------------------------------

namespace logger {

    enum level {
        trace,
        debug,
        info,
        error,
        none,
    };

    void setLevel(level loglevel, bool logdump);
    void registerWorkerThread(int no);
    bool isLogEnabled(level level);
    void print(char level, ansi::color color, std::ostringstream const & oss);

    class dump {
    public:
        dump(ansi::color color, char const * prefix);
        ~dump();

        void write(void const * data, size_t length);

    private:
        ansi::color     color_;
        char const    * prefix_;
        uint8_t         pending_[16];
        size_t          count_;

        void emithex(uint8_t const * data, size_t length);
    };
}

//--------------------------------------------------------------

#endif

//========================================================================
