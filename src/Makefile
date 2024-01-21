.PHONY: all clean install

all: input.o

input.o: input.c input.h

clean:
	- rm *.o
	- rm *.a

libinput.a: input.o
	ar cr $@ input.o

install: libinput.a input.h
	cp libinput.a $(DESTDIR)/lib/
	cp input.h $(DESTDIR)/include/
