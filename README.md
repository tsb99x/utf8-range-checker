Author: Anton Muravev

Homepage: https://tsb99x.ru

Filename: utf8-range-checker.c

Version: 2

Release Date: 2024-11-06T13:58:52+03:00

Last Update: 2025-02-10T21:06:20+03:00

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
