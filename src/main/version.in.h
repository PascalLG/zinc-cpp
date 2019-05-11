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

#ifndef __VERSION_H__
#define __VERSION_H__

//--------------------------------------------------------------
// Version number and other build information. This file is
// preprocessed by cmake to write actual values in place of the
// tags below.
//--------------------------------------------------------------

#define ZINC_VERSION_MAJOR       @ZINC_VERSION_MAJOR@
#define ZINC_VERSION_MINOR       @ZINC_VERSION_MINOR@
#define ZINC_PLATEFORM_NAME      "@CMAKE_SYSTEM_NAME@"
#define ZINC_BUILD_TIMESTAMP     @ZINC_BUILD_TIMESTAMP@

//--------------------------------------------------------------

#endif

//========================================================================
