cmake_minimum_required(VERSION 2.6)
project(FileHandler)

set(FILEHANDLER_SERIES 0)
set(FILEHANDLER_MAJOR  0)
set(FILEHANDLER_MINOR  0)

if(64BIT)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpic")
endif()

add_subdirectory(far)
add_subdirectory(iff)
add_subdirectory(libexpat)
add_subdirectory(libjpeg-turbo)
add_subdirectory(libmpg123)
add_subdirectory(libpng)
add_subdirectory(utk)
add_subdirectory(xa)
add_subdirectory(zlib)

set(FILEHANDLER_SOURCES
    Audio.cpp
    File.cpp
    Image.cpp
    bmp/read_bmp.c
    cst/cst.c
    utk/read_utk.c
    wav/read_wav.c
    xa/read_xa.c
)
if(WIN32)
    set(FILEHANDLER_SOURCES ${FILEHANDLER_SOURCES} resource.rc)
endif()

include_directories(${CMAKE_SOURCE_DIR}/Libraries/FileHandler)

#### Static library (uncomment to build)
#add_library(FileHandler_static STATIC ${FILEHANDLER_SOURCES})
#set_target_properties(FileHandler_static PROPERTIES
#    OUTPUT_NAME "FileHandler${FILEHANDLER_SERIES}"
#    PREFIX ""
#    CLEAN_DIRECT_OUTPUT 1)

add_library(FileHandler_shared SHARED ${FILEHANDLER_SOURCES})
set_target_properties(FileHandler_shared PROPERTIES
    OUTPUT_NAME "FileHandler${FILEHANDLER_SERIES}"
    VERSION ${FILEHANDLER_SERIES}${FILEHANDLER_MAJOR}.${FILEHANDLER_MINOR}.0
    SOVERSION ${FILEHANDLER_SERIES}${FILEHANDLER_MAJOR}
    PREFIX ""
    IMPORT_PREFIX ""
    CLEAN_DIRECT_OUTPUT 1)

target_link_libraries(FileHandler_shared far_static iff_static jpegturbo_static libmpg123_static libpng_static zlib_static m)
