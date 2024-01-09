LIBS = `pkg-config sdl2 --libs`
ifdef USE_CONFIGURATION
	LIBS += -lconfiguration
endif

all: example

example: example.c src/input.c
	$(CC) -g example.c src/input.c $(LIBS) -o $@

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
