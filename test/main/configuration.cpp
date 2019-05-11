//========================================================================
// Zinc - Unit Testing
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

#include "gtest/gtest.h"
#include "misc/string.h"
#include "main/configuration.h"

//--------------------------------------------------------------
// Test the default values of most config parameters.
//--------------------------------------------------------------

TEST(Config, DefaultValues) {
    Configuration cfg;

    std::vector<std::string> indexes =  {
        "index.html",
        "index.xhtml",
        "index.htm",
        "index.php",
        "index.py",
    };

    EXPECT_EQ(cfg.getListeningPort(),       8080                );
    EXPECT_EQ(cfg.getLimitThreads(),        4                   );
    EXPECT_EQ(cfg.getLimitRequestLine(),    2048                );
    EXPECT_EQ(cfg.getLimitRequestHeaders(), 8192                );
    EXPECT_EQ(cfg.getLimitRequestBody(),    32 * 1024 * 1024    );
    EXPECT_EQ(cfg.isCompressionEnabled(),   true                );
    EXPECT_EQ(cfg.getDirectoryIndexes(),    indexes             );
    EXPECT_EQ(cfg.isListingEnabled(),       true                );
    EXPECT_EQ(cfg.getTimeout(),             30                  );
    EXPECT_EQ(cfg.getExpires(),             3600                );

    EXPECT_EQ(cfg.getInterpreter("foo.php")->getSectionName(),  "PHP"    );
    EXPECT_EQ(cfg.getInterpreter("foo.php7")->getSectionName(), "PHP"    );
    EXPECT_EQ(cfg.getInterpreter("foo.py")->getSectionName(),   "Python" );

    EXPECT_EQ(cfg.getInterpreter("foo.html"), nullptr );
    EXPECT_EQ(cfg.getInterpreter("foo.txt"),  nullptr );
}

//--------------------------------------------------------------
// Test the setListeningPort function.
//--------------------------------------------------------------

TEST(Config, ListeningPort) {
    Configuration cfg;

    EXPECT_EQ(cfg.getListeningPort(), 8080  );
    cfg.setListeningPort(1024);
    EXPECT_EQ(cfg.getListeningPort(), 1024  );
    cfg.setListeningPort(65534);
    EXPECT_EQ(cfg.getListeningPort(), 65534 );
}

//--------------------------------------------------------------
// Test the save() function.
//--------------------------------------------------------------

TEST(Config, Save) {
    Configuration cfg;
    std::ostringstream ss;
    cfg.save(ss);

    std::vector<std::string> lines;
    string::split(ss.str(), '\n', 0, string::trim_both, [&] (std::string const & x) { 
        if (x.compare(0, 11, "Interpreter") != 0 && x.compare(0, 6, "Server") != 0) { // skip "Interpreter = " and "Server* =" directives, as they depend on the system configuration
            lines.push_back(x);
        }
        return true;
    });

    std::vector<std::string> ref = {
        "###############################",
        "#   Zinc configuration file   #",
        "###############################",
        "[Server]",
        "Listen = 8080",
        "LimitThreads = 4",
        "LimitRequestLine = 2048",
        "LimitRequestHeaders = 8192",
        "LimitRequestBody = 33554432",
        "Compression = yes",
        "DirectoryIndex = index.html index.xhtml index.htm index.php index.py",
        "DirectoryListing = yes",
        "Timeout = 30",
        "Expires = 3600",
        "[PHP]",
        "Extensions = php php7",
        "CmdLine =",
        "[Python]",
        "Extensions = py",
        "CmdLine =",
    };

    EXPECT_EQ(lines, ref);
}

//--------------------------------------------------------------
// Test the load() function.
//--------------------------------------------------------------

TEST(Config, Load) {
    std::string ref =
        "[Server]\n"
        "Listen = 2000\n"
        "LimitThreads = 15\n"
        "LimitRequestLine = 9999\n"
        "LimitRequestHeaders = 8888\n"
        "LimitRequestBody = 7777777\n"
        "Compression = NO\n"
        "DirectoryIndex = foo.html foo.foo\n"
        "DirectoryListing = false\n"
        "Timeout = 60\n"
        "Expires = 7200\n"
        "ServerAdmin = admin@test.com\n"
        "ServerName = www.test.com\n"
        "\n"
        "[Foo]\n"
        "Extensions = foo\n"
        "Interpreter = /usr/bin/foo\n"
        "CmdLine = -x\n"
        "\n";

    Configuration cfg;
    std::istringstream ss(ref);
    cfg.load(ss, "zinc.ini");

    EXPECT_EQ(cfg.getListeningPort(),       2000                                                );
    EXPECT_EQ(cfg.getLimitThreads(),        15                                                  );
    EXPECT_EQ(cfg.getLimitRequestLine(),    9999                                                );
    EXPECT_EQ(cfg.getLimitRequestHeaders(), 8888                                                );
    EXPECT_EQ(cfg.getLimitRequestBody(),    7777777                                             );
    EXPECT_EQ(cfg.isCompressionEnabled(),   false                                               );
    EXPECT_EQ(cfg.getDirectoryIndexes(),    std::vector<std::string>({"foo.html", "foo.foo"})   );
    EXPECT_EQ(cfg.isListingEnabled(),       false                                               );
    EXPECT_EQ(cfg.getTimeout(),             60                                                  );
    EXPECT_EQ(cfg.getExpires(),             7200                                                );
    EXPECT_EQ(cfg.getServerAdmin(),         "admin@test.com"                                    );
    EXPECT_EQ(cfg.getServerName(),          "www.test.com"                                      );

    Configuration::CGI const * cgi = cfg.getInterpreter("test.foo");
    EXPECT_EQ(cgi->getSectionName(),    "Foo");
    EXPECT_EQ(cgi->getInterpreter(),    "/usr/bin/foo");
    EXPECT_EQ(cgi->getCmdLine(),        "-x");
}

//========================================================================
