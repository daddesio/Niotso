cmake_minimum_required(VERSION 2.6)
project(far)

set(FAR_SOURCES
    far.c
    refpack_dec.c
)

add_library(far_static STATIC ${FAR_SOURCES})
set_target_properties(far_static PROPERTIES
    OUTPUT_NAME "far"
    CLEAN_DIRECT_OUTPUT 1)

#### Shared library (uncomment to build)
#add_library(far_shared SHARED ${FAR_SOURCES})
#set_target_properties(far_shared PROPERTIES
#    OUTPUT_NAME "far"
#    PREFIX ""
#    CLEAN_DIRECT_OUTPUT 1)

add_executable(farextract farextract.c)
target_link_libraries(farextract far_static)