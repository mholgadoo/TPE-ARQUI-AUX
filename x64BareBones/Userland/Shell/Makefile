include ../Makefile.inc

MODULE=0002-shell.bin
LIBOBJ=shell.o
MAINOBJ=shell_main.o
LOADEROBJ=_loader.o

all: $(MODULE)

$(MODULE): $(MAINOBJ) $(LOADEROBJ)
	$(GCC) $(GCCFLAGS) -I../include -T shell.ld $(LOADEROBJ) $(MAINOBJ) -o ../$(MODULE)

$(MAINOBJ): shell.c
	$(GCC) $(GCCFLAGS) -I../include -c $< -o $@

$(LIBOBJ): shell.c
	$(GCC) $(GCCFLAGS) -DSHELL_LIBRARY -I../include -c $< -o $@

$(LOADEROBJ): _loader.c
	$(GCC) $(GCCFLAGS) -I../include -c $< -o $@

clean:
	rm -f *.o ../$(MODULE)

.PHONY: all clean
