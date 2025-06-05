include ../Makefile.inc

MODULE=0000-sampleCodeModule.bin
SOURCES=$(wildcard [^_]*.c) ../Shell/shell.c

all: $(MODULE)

$(MODULE): $(SOURCES)
	$(GCC) $(GCCFLAGS) -DSHELL_LIBRARY -I../include -T sampleCodeModule.ld _loader.c $(SOURCES) -o ../$(MODULE)

clean:
	rm -rf *.o

.PHONY: all clean print
