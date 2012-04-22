#########################################
#### CMake generator file for Niotso ####

cmake_minimum_required(VERSION 2.6)

project(Niotso)

# Installation directory
set(CMAKE_INSTALL_PREFIX "c:/Program Files (x86)/Maxis/The Sims Online/Niotso" CACHE FILEPATH "Installation directory")

# Build type
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build configurations: Release Debug Release-MakeProfile Release-UseProfile")
else()
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build configurations: Release Debug")
endif()


if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    # Base options
    set(CFLAGS  "-Wall -Wextra -Wabi -pedantic -m32 -mmmx -msse -msse2 -msse3 -mfpmath=both -msahf")
    set(LDFLAGS "-m32")
    set(RCFLAGS "-F pe-i386")
    set(ASMFLAGS "-O3 -F win32")

    set(CFLAGS_LANG_C   "-ansi")
    set(CFLAGS_LANG_CPP "-fno-exceptions -fno-rtti -fno-threadsafe-statics")


    ####
    ## [Profiles]

    if(NOT (CMAKE_BUILD_TYPE MATCHES "Debug"))
        if(CMAKE_BUILD_TYPE MATCHES "Release-MakeProfile")
            set(CFLAGS "${CFLAGS} -fprofile-generate")
            set(LDFLAGS "${LDFLAGS} -lgcov")
        elseif(CMAKE_BUILD_TYPE MATCHES "Release-UseProfile")
            set(CFLAGS "${CFLAGS} -fprofile-use")
        endif()

        # Size
        set(CFLAGS_SIZE "${CFLAGS} -Os -g0 -fomit-frame-pointer -ffast-math -fmerge-all-constants -funsafe-loop-optimizations -fmerge-all-constants -fsched-pressure")
        set(LDFLAGS_SIZE "${LDFLAGS} -s -fwhole-program")

        # Speed
        set(CFLAGS_SPEED "${CFLAGS} -O3 -g0 -fomit-frame-pointer -ffast-math -fmerge-all-constants -funsafe-loop-optimizations -fmerge-all-constants -fsched-pressure -fmodulo-sched -fmodulo-sched-allow-regmoves -fgcse-sm -fgcse-las -fsched-spec-load -fsched-spec-load-dangerous -fsched-stalled-insns=0 -fsched-stalled-insns-dep -fsched2-use-superblocks -fipa-pta -fipa-matrix-reorg -ftree-loop-linear -floop-interchange -floop-strip-mine -floop-block -fgraphite-identity -floop-parallelize-all -ftree-loop-distribution -ftree-loop-im -ftree-loop-ivcanon -fivopts -fvect-cost-model -fvariable-expansion-in-unroller -fbranch-target-load-optimize -maccumulate-outgoing-args -flto")
        set(LDFLAGS_SPEED "${LDFLAGS} -s -fwhole-program -flto")
    else()
        # Debug
        set(CFLAGS_DEBUG "${CFLAGS} -O0 -g3")
        set(LDFLAGS_DEBUG "${LDFLAGS}")
            set(CFLAGS_SIZE "${CFLAGS_DEBUG}")
            set(LDFLAGS_SIZE "${LDFLAGS_DEBUG}")
            set(CFLAGS_SPEED "${CFLAGS_DEBUG}")
            set(LDFLAGS_SPEED "${LDFLAGS_DEBUG}")
    endif()

    set(CMAKE_C_FLAGS "${CFLAGS_LANG_C} ${CFLAGS_SIZE}")
    set(CMAKE_CXX_FLAGS "${CFLAGS_LANG_CPP} ${CFLAGS_SIZE}")
    set(CMAKE_SHARED_LINKER_FLAGS "${LDFLAGS} ${LDFLAGS_SIZE}")
    set(CMAKE_EXE_LINKER_FLAGS "${LDFLAGS} ${LDFLAGS_SIZE}")
    set(CMAKE_RC_FLAGS "${RCFLAGS}")
endif()

enable_language(ASM)
set(CMAKE_ASM_COMPILER "gcc")
set(CMAKE_ASM_FLAGS "-fno-strict-aliasing ${CFLAGS}")

if(WIN32)
    set(DIST_NAME "windows" CACHE STRING "Output folder name for the _dist folder (no start or end slash)")
elseif(APPLE)
    set(DIST_NAME "mac" CACHE STRING "Output folder name for the _dist folder (no start or end slash)")
elseif(UNIX)
    set(DIST_NAME "linux" CACHE STRING "Output folder name for the _dist folder (no start or end slash)")
else()
    set(DIST_NAME "unknown" CACHE STRING "Output folder name for the _dist folder (no start or end slash)")
endif()

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/_dist/${DIST_NAME}") (-flto means our archive files should not be redistributed)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/_dist/${DIST_NAME}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/_dist/${DIST_NAME}")

add_subdirectory(Client)
add_subdirectory(Libraries)
add_subdirectory(Server)
add_subdirectory(Tools)