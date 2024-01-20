CC=$(CROSS)gcc
PKG_CONFIG=$(CROSS)pkg-config
CFLAGS=-g -Wall
CFLAGS += `$(PKG_CONFIG) sdl2 --cflags`
LIBS = `$(PKG_CONFIG) sdl2 --libs`
ifdef USE_CONFIGURATION
	LIBS += -lconfiguration
	CFLAGS += -DUSE_CONFIGURATION
endif

.PHONY: all clean install test test_clean

all: example

example: example.c src/input.o
	$(CC) $(CFLAGS) example.c src/input.o $(LIBS) -o $@

install:
	$(MAKE) --directory src $@

#delete compiled binaries
clean:
	$(MAKE) --directory src $@
	- rm example

#buid and run tests
test:
	$(MAKE) --directory test $@

test_clean:
	$(MAKE) --directory test $@

