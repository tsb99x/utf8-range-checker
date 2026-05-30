.DELETE_ON_ERROR:
.SUFFIXES:

CC = gcc
CFLAGS = -O2 -std=c90 -Wall -Wextra -Wpedantic
BIN = utf8-range-checker

all: $(BIN)

clean:
	rm -f $(BIN)

.PHONY: all clean

%: %.c
	$(CC) $(CFLAGS) $< -o $@
