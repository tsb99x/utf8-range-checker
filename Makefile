.DELETE_ON_ERROR:

CC = gcc
CFLAGS = -O2 -std=c90 -Wall -Wextra -Wpedantic

all: utf8-range-checker

clean:
	rm -f utf8-range-checker

.PHONY: all clean
