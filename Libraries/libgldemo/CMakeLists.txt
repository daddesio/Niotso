cmake_minimum_required(VERSION 2.6)
project(libgldemo)

if(WIN32)
    set(LIBGLDEMO_SOURCES wgl.c)
else()
    set(LIBGLDEMO_SOURCES glx.c)
    add_definitions(-D_POSIX_C_SOURCE=200112)
endif()

add_library(libgldemo_static STATIC ${LIBGLDEMO_SOURCES})
set_target_properties(libgldemo_static PROPERTIES
    OUTPUT_NAME "gldemo"
    CLEAN_DIRECT_OUTPUT 1)
