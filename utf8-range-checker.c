/*

Author: Anton Muravev
Homepage: https://tsb99x.ru
Filename: utf8-range-checker.c
Version: 1
Release Date: 2024-11-06T13:58:52+03:00
License: Public Domain <https://unlicense.org>, see at the end of file.
Third-Party Dependencies: none

Compiled successfully with:

        GCC 12
        $ gcc -O2 -std=c90 \
                -Wall -Wextra -Wpedantic \
                -o utf8-range-checker{,.c}

        Clang 14
        $ clang -O2 -std=c90 \
                -Weverything \
                -o utf8-range-checker{,.c}

        MSVC 2010
        > cl.exe /O2 /W4 /Za utf8-range-checker.c

This program will check whether supplied STDIN contains ONLY Unicode code
points of specified ranges. Only UTF-8 encoding is supported.

If any code point OUTSIDE of specified ranges will be found, it's U+XXXX form
would be sent to STDERR. Error summary would be presented at the end of the
program execution if any error has occurred.

There are many reasons as to why you would need to know such nuances. Mine is
to check whether a web font would be rendered correctly on my website.

WARNING: DO NOT USE THIS PROGRAM FOR UTF-8 VALIDATION!
STDIN assumed to be always valid!

Limitations:
- Number of ranges is limited to 32 elements.

Use like this:

        $ cat index.html | ./utf8-range-checker U+0000-00FF U+0400-04FF

        $ find www/ -type f -name '*.html' -exec cat {} \; \
                | ./utf8-range-checker U+0000-00FF U+0400-04FF

An explanation of UTF-8 encoding for Unicode can be found at:
- https://www.unicode.org/versions/Unicode16.0.0/core-spec/chapter-3/#G7404

*/

#include <stdio.h>
#include <stdlib.h>

static const unsigned char FULL_BYTE = 0xFF;        /* 0b11111111 */
static const unsigned char NEXT_BYTE_MASK = 0xC0;   /* 0b11000000 */
static const unsigned char NEXT_BYTE_PREFIX = 0x80; /* 0b10000000 */

/* code-point size :                   1     2     3     4   */
static const unsigned char FBMASK[] = {0x80, 0xE0, 0xF0, 0xF8};
/* {0b10000000, 0b11100000, 0b11110000, 0b11111000} */
static const unsigned char PREFIX[] = {0x00, 0xC0, 0xE0, 0xF0};
/* {0b00000000, 0b11000000, 0b11100000, 0b11110000} */

static const char *ERR_EOF_ON_READING_NEXT_BYTE = /**/
    "ERR_EOF_ON_READING_NEXT_BYTE";
static const char *ERR_NEXT_BYTE_WRONG_PREFIX = /**/
    "ERR_NEXT_BYTE_WRONG_PREFIX";
static const char *ERR_NO_SUCH_CODE_POINT_EXISTS = /**/
    "ERR_NO_SUCH_CODE_POINT_EXISTS";
static const char *ERR_FAILED_TO_READ_RANGE = /**/
    "ERR_FAILED_TO_READ_RANGE";
static const char *ERR_TOO_MANY_RANGES = /**/
    "ERR_TOO_MANY_RANGES";

static void error(const char *m)
{
        fprintf(stderr, "Error: %s\n", m);
        exit(EXIT_FAILURE);
}

static unsigned char read_next_byte(FILE *file)
{
        int c = getc(file);

        if (c == EOF) {
                error(ERR_EOF_ON_READING_NEXT_BYTE);
        }
        if ((c & NEXT_BYTE_MASK) != NEXT_BYTE_PREFIX) {
                error(ERR_NEXT_BYTE_WRONG_PREFIX);
        }
        return c & (FULL_BYTE - NEXT_BYTE_MASK);
}

static unsigned long
complete_code_point(FILE *file, unsigned long code_point, int total_byte_count)
{
        int i;

        for (i = 0; i < total_byte_count - 1; i++) {
                code_point = code_point << 6 | read_next_byte(file);
        }
        return code_point;
}

static int find_code_point_byte_count(int first_byte)
{
        int i;

        for (i = 0; i < 4; i++) {
                if ((first_byte & FBMASK[i]) == PREFIX[i]) {
                        return i + 1;
                }
        }
        error(ERR_NO_SUCH_CODE_POINT_EXISTS);
        return -1;
}

struct range {
        unsigned long from;
        unsigned long to;
};

static int ranges_include(struct range ranges[],
                          int ranges_count,
                          unsigned long code_point)
{
        int i;

        for (i = 0; i < ranges_count; i++) {
                if (code_point >= ranges[i].from
                    && code_point <= ranges[i].to) {
                        return 1;
                }
        }
        return 0;
}

static int process_range_args(int argc, char *argv[], struct range ranges[])
{
        int range_cur = 0;

        if (argc > 33) {
                error(ERR_TOO_MANY_RANGES);
        }

        while (argc-- > 1) {
                int n = sscanf(argv[argc],
                               "U+%lX-%lX",
                               &ranges[range_cur].from,
                               &ranges[range_cur].to);
                if (n != 2) {
                        error(ERR_FAILED_TO_READ_RANGE);
                }
                range_cur++;
        }

        return range_cur;
}

int main(int argc, char *argv[])
{
        struct range ranges[32] = {0};
        int ranges_count = process_range_args(argc, argv, ranges);
        int out_of_range = 0;
        int ch = 0;

        while ((ch = getc(stdin)) != EOF) {
                int cp_byte_count = find_code_point_byte_count(ch);
                unsigned long cp = ch & (FULL_BYTE - FBMASK[cp_byte_count - 1]);
                cp = complete_code_point(stdin, cp, cp_byte_count);

                if (!ranges_include(ranges, ranges_count, cp)) {
                        out_of_range++;
                        fprintf(stderr, "U+%04lX\n", cp);
                }
        }

        if (ferror(stdin)) {
                perror("failed to execute getc(stdin)");
                return EXIT_FAILURE;
        }

        if (out_of_range > 0) {
                fprintf(stderr,
                        "total code points outside of specified ranges: %d\n",
                        out_of_range);
                return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
}

/*

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.

In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>

*/
