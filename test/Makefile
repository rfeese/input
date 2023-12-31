SHELL=/bin/sh
CC=$(CROSS)gcc
PKG_CONFIG=$(CROSS)pkg-config
CFLAGS=-g -Wall
UNITY=../../Unity/src/unity.c

.PHONY: all test clean

# default - run tests
all test:  test_input
	-./test_input


# build a test
test_input: $(UNITY) test_input.c ../src/input.h ../src/input.c
	$(CC) $(CFLAGS) $(UNITY) -fno-builtin-printf test_input.c ../src/input.c -o test_input

# delete compiled binaries
clean test_clean:
	- rm test_input
