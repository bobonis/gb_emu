IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR) -O4

ODIR=obj
LDIR =../lib

LIBS=-lSDLmain -lSDL2 -lSDL_image -lGL

DEPS = rom.h display.h gpu.h memory.h cpu.h timers.h interrupts.h input.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = rom.o display.o gpu.o memory.o cpu.o timers.o interrupts.o main.o input.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

gb_emu: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 