IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj
LDIR =../lib

LIBS=

DEPS = memory.h rom.h cpu.h gpu.h timers.h interrupts.c
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = gpu.o memory.o rom.o cpu.o main.o timers.o interrupts.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

gb_emu: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
