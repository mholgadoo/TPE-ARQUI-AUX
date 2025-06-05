include ../Makefile.inc

MODULE=0000-sampleCodeModule.bin
SOURCES=$(wildcard [^_]*.c)

all: $(MODULE)

$(MODULE): $(SOURCES) ../Shell/shell.o
	$(GCC) $(GCCFLAGS) -I../include -T sampleCodeModule.ld _loader.c $(SOURCES) ../Shell/shell.o -o ../$(MODULE)

clean:
	rm -f *.o ../$(MODULE)

.PHONY: all clean
