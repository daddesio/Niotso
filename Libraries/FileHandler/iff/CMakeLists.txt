cmake_minimum_required(VERSION 2.6)
project(iff)

set(IFF_SOURCES
    cats.c
    iff.c
    bcon.c
    fcns.c
    glob.c
    palt.c
    rsmp.c
    str.c
    string.c
    tmpl.c
    trcn.c
)

add_library(iff_static STATIC ${IFF_SOURCES})
set_target_properties(iff_static PROPERTIES
    OUTPUT_NAME "iff"
    PREFIX ""
    CLEAN_DIRECT_OUTPUT 1)

#### Shared library (uncomment to build)
#add_library(iff_shared SHARED ${IFF_SOURCES})
#set_target_properties(iff_shared PROPERTIES
#    OUTPUT_NAME "iff"
#    PREFIX ""
#    CLEAN_DIRECT_OUTPUT 1)

add_executable(iffexport iffexport.c)
target_link_libraries(iffexport iff_static)