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
#include "misc/filesys.h"

//--------------------------------------------------------------
// Test filepath creation.
//--------------------------------------------------------------

TEST(FilePath, Creation) {
    EXPECT_EQ(fs::filepath("test.txt").getStdString(),                  "test.txt"  );
    EXPECT_EQ(fs::filepath("a/b").getStdString(),                       "a/b"       );
    EXPECT_EQ(fs::filepath("a/b/c").getStdString(),                     "a/b/c"     );

    EXPECT_STREQ(fs::filepath("test.txt").getCString(),                 "test.txt"  );
    EXPECT_STREQ(fs::filepath("a/b").getCString(),                      "a/b"       );
    EXPECT_STREQ(fs::filepath("a/b/c").getCString(),                    "a/b/c"     );
}

//--------------------------------------------------------------
// Test the getExtension() function.
//--------------------------------------------------------------

TEST(FilePath, getExtension) {
    EXPECT_EQ(fs::filepath("test").getExtension(),                      ""      );
    EXPECT_EQ(fs::filepath("test.").getExtension(),                     "."     );
    EXPECT_EQ(fs::filepath("test.tar").getExtension(),                  ".tar"  );
    EXPECT_EQ(fs::filepath("test.tar.gz").getExtension(),               ".gz"   );
    EXPECT_EQ(fs::filepath(".conf").getExtension(),                     ".conf" );

    EXPECT_EQ(fs::filepath("/test").getExtension(),                     ""      );
    EXPECT_EQ(fs::filepath("/test.").getExtension(),                    "."     );
    EXPECT_EQ(fs::filepath("/test.tar").getExtension(),                 ".tar"  );
    EXPECT_EQ(fs::filepath("/test.tar.gz").getExtension(),              ".gz"   );
    EXPECT_EQ(fs::filepath("/.conf").getExtension(),                    ".conf" );

    EXPECT_EQ(fs::filepath("foo/test").getExtension(),                  ""      );
    EXPECT_EQ(fs::filepath("foo/test.").getExtension(),                 "."     );
    EXPECT_EQ(fs::filepath("foo/test.tar").getExtension(),              ".tar"  );
    EXPECT_EQ(fs::filepath("foo/test.tar.gz").getExtension(),           ".gz"   );
    EXPECT_EQ(fs::filepath("foo/.conf").getExtension(),                 ".conf" );

    EXPECT_EQ(fs::filepath("/bla/foo/test").getExtension(),             ""      );
    EXPECT_EQ(fs::filepath("/bla/foo/test.").getExtension(),            "."     );
    EXPECT_EQ(fs::filepath("/bla/foo/test.tar").getExtension(),         ".tar"  );
    EXPECT_EQ(fs::filepath("/bla/foo/test.tar.gz").getExtension(),      ".gz"   );
    EXPECT_EQ(fs::filepath("/bla/foo/.conf").getExtension(),            ".conf" );

    EXPECT_EQ(fs::filepath("/bla/foo.d/test").getExtension(),           ""      );
    EXPECT_EQ(fs::filepath("/bla/foo.d/test.").getExtension(),          "."     );
    EXPECT_EQ(fs::filepath("/bla/foo.d/test.tar").getExtension(),       ".tar"  );
    EXPECT_EQ(fs::filepath("/bla/foo.d/test.tar.gz").getExtension(),    ".gz"   );
    EXPECT_EQ(fs::filepath("/bla/foo.d/.conf").getExtension(),          ".conf" );

    EXPECT_EQ(fs::filepath("/bla.d/foo/test").getExtension(),           ""      );
    EXPECT_EQ(fs::filepath("/bla.d/foo/test.").getExtension(),          "."     );
    EXPECT_EQ(fs::filepath("/bla.d/foo/test.tar").getExtension(),       ".tar"  );
    EXPECT_EQ(fs::filepath("/bla.d/foo/test.tar.gz").getExtension(),    ".gz"   );
    EXPECT_EQ(fs::filepath("/bla.d/foo/.conf").getExtension(),          ".conf" );
}

//--------------------------------------------------------------
// Test the getLastComponent() function.
//--------------------------------------------------------------

TEST(FilePath, getLastComponent) {
#ifdef _WIN32
#else
    EXPECT_EQ(fs::filepath("/directory/file.ext").getLastComponent(),  "file.ext");
    EXPECT_EQ(fs::filepath("/foo").getLastComponent(),                 "foo");
    EXPECT_EQ(fs::filepath("foo").getLastComponent(),                  "foo");
    EXPECT_EQ(fs::filepath("foo/").getLastComponent(),                 "");
#endif
}

//--------------------------------------------------------------
// Test the getDirectory() function.
//--------------------------------------------------------------

TEST(FilePath, getDirectory) {
#ifdef _WIN32
#else
    EXPECT_EQ(fs::filepath("/directory/other.ext").getDirectory(),  "/directory");
    EXPECT_EQ(fs::filepath("foo").getDirectory(),                   ".");
    EXPECT_EQ(fs::filepath("/").getDirectory(),                     "/");
    EXPECT_EQ(fs::filepath("/foo").getDirectory(),                  "/");
    EXPECT_EQ(fs::filepath("/foo/bar/baz").getDirectory(),          "/foo/bar");
    EXPECT_EQ(fs::filepath("/foo/bar/baz/").getDirectory(),         "/foo/bar/baz");
    EXPECT_EQ(fs::filepath("foo/bar/baz").getDirectory(),           "foo/bar");
#endif
}

//--------------------------------------------------------------
// Test the isAbsolute() function.
//--------------------------------------------------------------

TEST(FilePath, isAbsolute) {
#ifdef _WIN32

#else
    EXPECT_FALSE(fs::filepath("").isAbsolute());
    EXPECT_FALSE(fs::filepath("foo").isAbsolute());
    EXPECT_FALSE(fs::filepath("foo/").isAbsolute());
    EXPECT_FALSE(fs::filepath("foo/bar").isAbsolute());
    EXPECT_FALSE(fs::filepath("foo/bar/").isAbsolute());
    EXPECT_TRUE (fs::filepath("/foo").isAbsolute());
    EXPECT_TRUE (fs::filepath("/foo/").isAbsolute());
    EXPECT_TRUE (fs::filepath("/foo/bar").isAbsolute());
    EXPECT_TRUE (fs::filepath("/foo/bar/").isAbsolute());
#endif
}

//--------------------------------------------------------------
// Test the += operator.
//--------------------------------------------------------------

TEST(FilePath, append) {
#ifdef _WIN32

#else
    EXPECT_EQ(fs::filepath("foo") + fs::filepath("bar"),        fs::filepath("foo/bar"));
    EXPECT_EQ(fs::filepath("foo/") + fs::filepath("bar"),       fs::filepath("foo/bar"));
    EXPECT_EQ(fs::filepath("foo") + fs::filepath("/bar"),       fs::filepath("/bar"));
    EXPECT_EQ(fs::filepath("foo") + fs::filepath("./bar"),      fs::filepath("foo/bar"));
    EXPECT_EQ(fs::filepath("foo") + fs::filepath("../bar"),     fs::filepath("foo/../bar"));
    EXPECT_EQ(fs::filepath("foo") + fs::filepath("././bar"),    fs::filepath("foo/bar"));
    EXPECT_EQ(fs::filepath("foo") + fs::filepath(".//.//bar"),  fs::filepath("foo/bar"));
    EXPECT_EQ(fs::filepath("foo") + fs::filepath("/"),          fs::filepath("/"));
    EXPECT_EQ(fs::filepath("foo") + fs::filepath(""),           fs::filepath("foo"));
    EXPECT_EQ(fs::filepath("foo") + fs::filepath("./"),         fs::filepath("foo"));
    EXPECT_EQ(fs::filepath("") + fs::filepath("bar"),           fs::filepath("./bar"));
    EXPECT_EQ(fs::filepath("") + fs::filepath("./bar"),         fs::filepath("./bar"));
    EXPECT_EQ(fs::filepath("") + fs::filepath("/bar"),          fs::filepath("/bar"));
#endif
}

//--------------------------------------------------------------
// Test the fromURI() function.
//--------------------------------------------------------------

TEST(FilePath, fromURI) {
#ifdef _WIN32
    EXPECT_EQ(fs::makeFilepathFromURI("index.html"),    ".\\index.html"  );
    EXPECT_EQ(fs::makeFilepathFromURI("/index.html"),   ".\\index.html"  );
    EXPECT_EQ(fs::makeFilepathFromURI("/a/b/"),         ".\\a\\b\\"      );
    EXPECT_EQ(fs::makeFilepathFromURI("/"),             ".\\"            );
    EXPECT_EQ(fs::makeFilepathFromURI(""),              ".\\"            );
#else
    EXPECT_EQ(fs::makeFilepathFromURI("index.html"),    "./index.html"  );
    EXPECT_EQ(fs::makeFilepathFromURI("/index.html"),   "./index.html"  );
    EXPECT_EQ(fs::makeFilepathFromURI("/a/b/"),         "./a/b/"        );
    EXPECT_EQ(fs::makeFilepathFromURI("/"),             "./"            );
    EXPECT_EQ(fs::makeFilepathFromURI(""),              "./"            );
#endif
}

//--------------------------------------------------------------
// Test the fs::tmpfile class.
//--------------------------------------------------------------

TEST(TmpFile, Creation) {
    fs::tmpfile f;
    EXPECT_TRUE(f.write("ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26));
    EXPECT_TRUE(f.write("0123456789", 10));
    EXPECT_EQ(f.getSize(), 36);
    EXPECT_TRUE(f.write("abcdefghijklmnopqrstuvwxyz", 26));
    EXPECT_EQ(f.getSize(), 62);

    char buffer[64];
    memset(buffer, 0, sizeof(buffer));
#ifdef _WIN32

#else
    EXPECT_EQ(lseek(f.getFileDescriptor(), 0, SEEK_SET), 0);
    EXPECT_EQ(read(f.getFileDescriptor(), buffer, 62),   62);
    EXPECT_STREQ(buffer, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz");
#endif
}

//========================================================================
