# -pedantic          
#

IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR) $(shell pkg-config --cflags --libs gtk+-3.0 epoxy) -O4 -fwrapv -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wsign-conversion -Wstrict-overflow=5 -Wswitch-default -Wundef -Wno-unused

ODIR=obj
LDIR =../lib

LIBS=$(shell pkg-config --libs gtk+-3.0) -lSDLmain -lSDL2 -lSDL_image -lGL -lm

DEPS = rom.h display.h gpu.h memory.h cpu.h timers.h interrupts.h input.h hardware.h window.h render.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = rom.o display.o gpu.o memory.o cpu.o timers.o interrupts.o main.o input.o hardware.o window.o render.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

gb_emu: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 