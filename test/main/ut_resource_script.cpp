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
#include "main/resource_script.h"

//--------------------------------------------------------------
// Test the buildArguments function.
//--------------------------------------------------------------

TEST(ResourceScript, buildArguments) {
    auto f = [] (char const * interpreter, char const * cmdline, char const * script) {
        Configuration::CGI cgi("test", "xxx", interpreter, cmdline);
        ResourceScript res(script, script, "pathinfo", cgi);
        return res.buildArguments();
    };

#ifdef _WIN32
    static char const * php = "c:\\foo\\php-cgi.exe";
    static char const * exe = php + 7;
#else
    static char const * php = "/usr/bin/php-cgi";
    static char const * exe = php + 9;
#endif

    EXPECT_EQ(f(php, "", "test.php"),                std::vector<std::string>({ exe, "test.php"                        }));
    EXPECT_EQ(f(php, "-x", "test.php"),              std::vector<std::string>({ exe, "-x", "test.php"                  }));
    EXPECT_EQ(f(php, "-xy", "test.php"),             std::vector<std::string>({ exe, "-xy", "test.php"                 }));
    EXPECT_EQ(f(php, "  -xy", "test.php"),           std::vector<std::string>({ exe, "-xy", "test.php"                 }));
    EXPECT_EQ(f(php, "-xy  ", "test.php"),           std::vector<std::string>({ exe, "-xy", "test.php"                 }));
    EXPECT_EQ(f(php, "  -xy  ", "test.php"),         std::vector<std::string>({ exe, "-xy", "test.php"                 }));
    EXPECT_EQ(f(php, "-xy opt", "test.php"),         std::vector<std::string>({ exe, "-xy", "opt", "test.php"          }));
    EXPECT_EQ(f(php, "  -xy  opt  ", "test.php"),    std::vector<std::string>({ exe, "-xy", "opt", "test.php"          }));
    EXPECT_EQ(f(php, "abc\\ def", "test.php"),       std::vector<std::string>({ exe, "abc def", "test.php"             }));
    EXPECT_EQ(f(php, "abc\\\\def", "test.php"),      std::vector<std::string>({ exe, "abc\\def", "test.php"            }));
    EXPECT_EQ(f(php, "-x \"a'b c\" d", "test.php"),  std::vector<std::string>({ exe, "-x", "a'b c", "d", "test.php"    }));
    EXPECT_EQ(f(php, "-x 'a\\b c' d", "test.php"),   std::vector<std::string>({ exe, "-x", "a\\b c", "d", "test.php"   }));
}

//========================================================================
