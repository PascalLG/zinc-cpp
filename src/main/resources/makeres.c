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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

//--------------------------------------------------------------
// Build variable name from source path.
//--------------------------------------------------------------

static void makeVarName(char * res, char const * path) {
    char const * p = path;
    char * q = res;
    int r;
    while ((r = *p++) != 0) {
        if (isalnum(r)) {
            *q++ = tolower(r);
        } else {
            *q++ = '_';
        }
    }
    *q = '\0';
}

//--------------------------------------------------------------
// Generate C++ source from file content.
//--------------------------------------------------------------

static void generateContent(FILE * fpin, FILE * fpout, char const * name) {
    fprintf(fpout, "static unsigned char const %s[] = {\n    ", name);

    int r, c = 0;
    while ((r = fgetc(fpin)) >= 0) {
        fprintf(fpout, "0x%02X, ", r);
        if (++c % 16 == 0) {
            fprintf(fpout, "\n    ");
        }
    }

    fprintf(fpout, "0x00\n};\n\n");
    fprintf(fpout, "static unsigned int const %s_length = %d;\n\n", name, c);
}

//--------------------------------------------------------------
// Print usage.
//--------------------------------------------------------------

static void usage(void) {
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "    makeres <source> <destination>\n\n");
}

//--------------------------------------------------------------
// Entry point.
//--------------------------------------------------------------

int main(int argc, char ** argv) {
    if (argc != 3) {
        usage();
        return EXIT_FAILURE;
    }

    char name[256];
    makeVarName(name, argv[1]);

    FILE * fpin = fopen(argv[1], "rb");
    if (!fpin) {
        fprintf(stderr, "error: file %s not found\n", argv[1]);
        return EXIT_FAILURE;
    }

    FILE * fpout = fopen(argv[2], "wb+");
    if (!fpout) {
        fprintf(stderr, "error: cannot create file %s\n", argv[2]);
        fclose(fpin);
        return EXIT_FAILURE;
    }

    generateContent(fpin, fpout, name);
    fclose(fpin);
    fclose(fpout);

    return EXIT_SUCCESS;
}

//========================================================================
