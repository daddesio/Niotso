#######################################
#### Makefile for compiling Niotso ####

# Environment (valid options: cmd posix)
buildenv = cmd

# Installation directory (do not use backslashes)
installdir = c:/Program Files (x86)/Maxis/The Sims Online/Niotso

# Debugging (valid options: yes no)
debug = no

# Profile-guided optimization (valid options: none instrument optimize)
pgo = none

CC = gcc
LD = gcc
AR = ar
RC = windres

# Base options
CFLAGS  = -m32 -mmx -sse -sse2 -sse3 -mfpmath=both -msahf
LDFLAGS = -m32 -static -static-libgcc
ARFLAGS = rcs
RCFLAGS = -F pe-i386

CFLAGS_LANG_C   = -ansi
CFLAGS_LANG_CPP = -std=c++98

####
## [Profiles]

# Size
CFLAGS_SIZE  = $(CFLAGS) -Os -g0 -fomit-frame-pointer -ffast-math -fmerge-all-constants -funsafe-loop-optimizations -fsched-pressure -fno-enforce-eh-specs
LDFLAGS_SIZE = $(LDFLAGS) -s

# Speed
CFLAGS_SPEED  = $(CFLAGS) -O3 -g0 -fomit-frame-pointer -ffast-math -fmerge-all-constants -funsafe-loop-optimizations -fsched-pressure -fno-enforce-eh-specs -fmodulo-sched -fmodulo-sched-allow-regmoves -fgcse-sm -fgcse-las -fsched-spec-load -fsched-spec-load-dangerous -fsched-stalled-insns=0 -fsched-stalled-insns-dep -fsched2-use-superblocks -fipa-pta -fipa-matrix-reorg -ftree-loop-linear -floop-interchange -floop-strip-mine -floop-block -fgraphite-identity -floop-parallelize-all -ftree-loop-distribution -ftree-loop-im -ftree-loop-ivcanon -fivopts -fvect-cost-model -fvariable-expansion-in-unroller -fbranch-target-load-optimize -maccumulate-outgoing-args -fwhole-program -flto
LDFLAGS_SPEED = $(LDFLAGS) -s -flto

# Debug
CFLAGS_DEBUG  = $(CFLAGS) -O0 -g3
LDFLAGS_DEBUG = $(LDFLAGS)

WARNINGS = -Wall -Wextra -Wabi -pedantic

ifeq($(debug), yes)
    CFLAGS_SIZE = $(CFLAGS_DEBUG)
    CFLAGS_SPEED = $(CFLAGS_DEBUG)
    LDFLAGS_SIZE = $(LDFLAGS_DEBUG)
    LDFLAGS_SPEED = $(LDFLAGS_DEBUG)
endif


####
## [Miscellaneous]

ifeq ($(buildenv),cmd)
    RM_R = -@del /s /q
    CP_F = copy /y
    EXE  = .exe
    DLL  = .dll
else
    RM_R = -@rm -r
    CP_F = cp -f
    EXE  =
    DLL  = .so
endif

ifeq ($(pgo),instrument)
    CFLAGS  += -fprofile-generate
    LDFLAGS += -lgcov
else ifeq ($(pgo),optimize)
    CFLAGS  += -fprofile-use
endif

niotso_buildsys = 1


####
## [Targets]

include packages.makefile

filter_obj = *.o *.lo *.la *.Plo *.Pla *.gch *.pch *.obj *.res *.exp *.dep *.aps *.intermediate.manifest
filter_exe = *.a *.lib $(niotso_exes) $(niotso_dlls)
filter_profile = *.gcda
filter_dist =  *.dsp *.dsw *.user *.ncb *.pdb *.idb *.opt *.plg *.suo BuildLog.htm

objclean:
	$(RM_R) $(filter_obj)
clean: objclean
	$(RM_R) $(filter_obj) $(filter_exe)
profileclean:
	$(RM_R) $(filter_profile)
distclean:
	$(RM_R) $(filter_obj) $(filter_exe) $(filter_profile) $(filter_dist)