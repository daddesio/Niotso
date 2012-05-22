# macros --------------------------------------------------------------------
CC = gcc
LD = gcc
CFLAGS = -m32 -Wall -Wextra -Wabi -Os -march=i686 -fomit-frame-pointer -ffast-math -funsafe-loop-optimizations -fmerge-all-constants -g0 -fno-exceptions
LDFLAGS = -m32 -s -fwhole-program

AR = ar rcs
WINDRES = windres -F pe-i386

LIBRARY_OBJS = obj/libfar.o obj/qfsdecompress.o
RESOURCE_OBJ = obj/resource.o
FAREXTRACT_OBJ = obj/farextract.o

# These will rebuild the entire library upon edit.
DEPS =	Makefile \
		config.h \
		include/libfar.h

# dependencies --------------------------------------------------------------
all: libfar.a libfar-10.dll farextract.exe

$(LIBRARY_OBJS): $(DEPS)

libfar.a: $(LIBRARY_OBJS)
	$(AR) $@ $(LIBRARY_OBJS)

libfar-10.dll: $(LIBRARY_OBJS) $(RESOURCE_OBJ)
	$(CC) $(LDFLAGS) -shared -o $@ $(LIBRARY_OBJS) $(RESOURCE_OBJ)

farextract.exe: libfar.a $(FAREXTRACT_OBJ)
	$(CC) $(LDFLAGS) -o $@ $(FAREXTRACT_OBJ) libfar.a

# make rules ----------------------------------------------------------------
obj/%.o: %.c
	$(CC) -c -ansi -pedantic $(CFLAGS) -o $@ $<

obj/%.o: %.rc
	$(WINDRES) -i $< -o $@

# maintenance ---------------------------------------------------------------
clean:
	del /Q /S obj libfar.a libfar-10.dll farextract.exe