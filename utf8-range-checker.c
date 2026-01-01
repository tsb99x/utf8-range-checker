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
static const char *ERR_INPUT_READ_ERROR = /**/
    "ERR_INPUT_READ_ERROR";

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

        if (!feof(stdin)) {
                error(ERR_INPUT_READ_ERROR);
        }

        if (out_of_range > 0) {
                fprintf(stderr,
                        "total code points outside of specified ranges: %d\n",
                        out_of_range);
                return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
}
