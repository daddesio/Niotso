cmake_minimum_required(VERSION 2.6)
project(iff2html)

set(IFF2HTML_SOURCES
    iff2html.c
    md5.c
    image.c
    opngreduc.c
    ../../Libraries/FileHandler/bmp/read_bmp.c
)

include_directories(${CMAKE_SOURCE_DIR}/Libraries/FileHandler)

add_executable(iff2html ${IFF2HTML_SOURCES})
target_link_libraries(iff2html iff_static libpng_static zlib_static)