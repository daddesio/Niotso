# macros --------------------------------------------------------------------
CC = gcc
LD = gcc
CFLAGS = -m32 -Os -march=i686 -fomit-frame-pointer -ffast-math -funsafe-loop-optimizations -fmerge-all-constants -g0 -fno-exceptions
LDFLAGS = -m32 -s -fwhole-program

WINDRES	= windres -F pe-i386

LIBS = -mwindows -lole32 -luxtheme -L. -lpng15

OBJS =	obj/windows/common.o \
		obj/windows/Startup.o \
		obj/windows/Interaction.o \
		obj/windows/MainWindow.o \
        obj/windows/ReadPNGIcon.o \
        obj/windows/Dialog/AddToArchive.o \
        obj/windows/Dialog/NewArchive.o \
        obj/windows/resource.o

# These will rebuild the entire program upon edit.
DEPS =	Makefile \
		FARDive.hpp \
		version.hpp \
		windows/GUI.hpp \
        windows/resource.hpp \
        windows/Windows.hpp

# dependencies --------------------------------------------------------------
all: ./FARDive.exe
./FARDive.exe: $(OBJS)
	$(LD) $(LDFLAGS) -L. -o $@ $(OBJS) $(LIBS)

$(OBJS): $(DEPS)

# make rules ----------------------------------------------------------------
./obj/%.o: %.cpp
	$(CC) -c $(CFLAGS) -o $@ $<

./obj/%.o: %.rc
	$(WINDRES) -i $< -o $@

# maintenance ---------------------------------------------------------------
clean:
	del /Q /S FARDive.exe obj