IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj
LDIR =../lib

LIBS=-lSDLmain -lSDL2 -lSDL_image -lGL

DEPS = display.h gpu.h memory.h rom.h cpu.h  timers.h interrupts.c
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = display.o gpu.o memory.o rom.o cpu.o timers.o interrupts.o main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

gb_emu: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
