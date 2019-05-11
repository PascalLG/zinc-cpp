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

#include <csignal>
#include <string>
#include <iostream>

#include "../misc/portability.h"
#include "../misc/logger.h"
#include "../http/http_server.h"
#include "zinc.h"

static std::unique_ptr<HttpServer>  server;                     // the actual server instance
static fs::filepath                 configFile  = "zinc.ini";   // name of the configuration file

#ifdef _WIN32
static char const                   optChar     = '/';          // Command line option delimiter for Win32
#else
static char const                   optChar     = '-';          // Command line option delimiter for Linux/macOS
#endif

//========================================================================

//--------------------------------------------------------------
// Handle interrupt and kill signals.
//--------------------------------------------------------------

static void handleTerminate(int signal) {
    server->stop();
}

//--------------------------------------------------------------
// Print usage.
//--------------------------------------------------------------

static void printUsage(std::ostream & out) {
    out << "usage: zinc [options]" << std::endl
        << "  " << optChar << "p <num>    Listen to port <num>" << std::endl
        << "  " << optChar << "c          Generate a " << configFile << " file" << std::endl
        << "  " << optChar << "n          Do not load the " << configFile << " file" << std::endl
        << "  " << optChar << "q          Suppress display of the banner" << std::endl
        << "  " << optChar << "l <level>  Set the log level to none/error/info/debug/trace" << std::endl
        << "  " << optChar << "b          Dump request and response bodies" << std::endl
        << "  " << optChar << "a          Do not emit ANSI sequences (monochrome output)" << std::endl
        << "  " << optChar << "h          Display this help screen" << std::endl;
}

//--------------------------------------------------------------
// Entry point.
//--------------------------------------------------------------

int main(int argc, char ** argv) {

    // Initialisation.

    tzset_();
    ansi::setEnabled(true);
    logger::registerWorkerThread(0);

    // Parse command line. (Not using getopt to ensure
    // portability on Win32.)

    bool genconf = false, ignoreconf = false, quiet = false, dump = false;
    logger::level log = logger::info;
    int ndx = 1, port = 0;

    while (ndx < argc && argv[ndx][0] == optChar) {
        std::string key = argv[ndx] + 1;
        if (key == "p") {
            if (++ndx >= argc) {
                printUsage(std::cerr);
                return EXIT_FAILURE;
            }
            char * err;
            port = strtol(argv[ndx], &err, 10);
            if (*err || port < 1024 || port > 65535) {
                std::cerr << "error: invalid port number" << std::endl;
                return EXIT_FAILURE;
            }
        }else if (key == "l") {
            if (++ndx >= argc) {
                printUsage(std::cerr);
                return EXIT_FAILURE;
            }
            std::string level = argv[ndx];
            if (level == "none") {
                log = logger::none;
            } else if (level == "error") {
                log = logger::error;
            } else if (level == "info") {
                log = logger::info;
            } else if (level == "debug") {
                log = logger::debug;
            } else if (level == "trace") {
                log = logger::trace;
            } else {
                std::cerr << "error: invalid log level " << level << std::endl;
                return EXIT_FAILURE;
            }
        } else if (key == "c") {
            genconf = true;
        } else if (key == "n") {
            ignoreconf = true;
        } else if (key == "q") {
            quiet = true;
        } else if (key == "b") {
            dump = true;
        } else if (key == "a") {
            ansi::setEnabled(false);
        } else if (key == "h") {
            printUsage(std::cout);
            return EXIT_SUCCESS;
        } else {
            std::cerr << "error: invalid option " << optChar << key << std::endl;
            return EXIT_FAILURE;
        }
        ndx++;
    }

    if (ndx < argc) {
        std::cerr << "error: invalid argument " << argv[ndx] << std::endl;
        return EXIT_FAILURE;
    }

    logger::setLevel(log, dump);

    // Handle configuration.

    Zinc & zinc = Zinc::getInstance();
    Configuration & configuration = zinc.getConfiguration();

    if (!ignoreconf) {
        if (!configuration.load(configFile)) {
            return EXIT_FAILURE;
        }
    }

    if (port > 0) {
        configuration.setListeningPort(port);
    }

    if (genconf) {
        bool ok = configuration.save(configFile);
        if (ok) {
            std::cout << "current configuration written to " << configFile << std::endl;
            return EXIT_SUCCESS;
        } else {
            std::cerr << "error: unable to write to " << configFile << std::endl;
            return EXIT_FAILURE;
        }
    }

    // Display banner and configuation information.

    if (!quiet) {
        std::cout << zinc.getVersionString() << " - Personal Web Server" << std::endl
                  << "(c) 2019 - Æquans" << std::endl
                  << std::endl
                  << "Configuration:" << std::endl;

        configuration.log();
    }

    // Install handlers.
#ifndef _WIN32
    signal(SIGINT,  handleTerminate);   // Ctrl+C
    signal(SIGHUP,  handleTerminate);   // console is closed
    signal(SIGTERM, handleTerminate);   // request for termination
    signal(SIGPIPE, SIG_IGN);           // ignore broken sockets
#endif
    // Instantiate and start the server.

    server = std::make_unique<HttpServer>(zinc);
    int status = server->startup();
    server.reset();

    return status;
}

//========================================================================
