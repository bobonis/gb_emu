# -pedantic   -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror -Wno-unused
#

IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR) -O4 -Wall -Wextra

ODIR=obj
LDIR =../lib

LIBS=-lSDLmain -lSDL2 -lSDL_image -lGL

DEPS = rom.h display.h gpu.h memory.h cpu.h timers.h interrupts.h input.h hardware.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = rom.o display.o gpu.o memory.o cpu.o timers.o interrupts.o main.o input.o hardware.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

gb_emu: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 