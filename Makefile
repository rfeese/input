#binaries
all: example

example: example.c src/input.c
	$(CC) -g example.c src/input.c -lSDL2 -o $@

#delete compiled binaries
clean:
	- rm src/*.o
	- rm example

#buid and run tests
test:
	$(MAKE) --directory test $@

test_clean:
	$(MAKE) --directory test $@

.PHONY: all clean test test_clean
