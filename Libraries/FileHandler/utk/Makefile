# macros --------------------------------------------------------------------
CC = gcc
LD = gcc
CFLAGS = -m32 -Wall -Wextra -Wabi -Os -march=i686 -fomit-frame-pointer -g0
LDFLAGS = -m32 -s -fwhole-program

AR = ar rcs
WINDRES = windres -F pe-i386

OBJECTS = obj/read_utk.o obj/utkdecode.o

# These will rebuild the entire library upon edit.
DEPS =	Makefile \
		read_utk.h

# dependencies --------------------------------------------------------------
all: utkdecode.exe

$(OBJECTS): $(DEPS)

utkdecode.exe: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

# make rules ----------------------------------------------------------------
obj/%.o: %.c
	$(CC) -c -ansi -pedantic $(CFLAGS) -o $@ $<

# maintenance ---------------------------------------------------------------
clean:
	del /Q /S obj utkdecode.exe