# macros --------------------------------------------------------------------
CC = gcc
LD = gcc
CFLAGS = -m32 -Wall -Wextra -Wabi -Os -march=i686 -fomit-frame-pointer -ffast-math -funsafe-loop-optimizations -fmerge-all-constants -g0 -fno-exceptions
LDFLAGS = -m32 -s -fwhole-program

AR = ar rcs
WINDRES = windres -F pe-i386

OBJECTS = obj/read_xa.o obj/xadecode.o

# These will rebuild the entire library upon edit.
DEPS =	Makefile \
		read_xa.h

# dependencies --------------------------------------------------------------
all: xadecode.exe

$(OBJECTS): $(DEPS)

xadecode.exe: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

# make rules ----------------------------------------------------------------
obj/%.o: %.c
	$(CC) -c -ansi -pedantic $(CFLAGS) -o $@ $<

# maintenance ---------------------------------------------------------------
clean:
	del /Q /S obj xadecode.exe