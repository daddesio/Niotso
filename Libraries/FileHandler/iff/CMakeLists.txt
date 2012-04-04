cmake_minimum_required(VERSION 2.6)
project(iff)

set(IFF_SOURCES
    bhav.c
    stbl.c
    sprite.c
    chunks.c
    iff.c
)

include_directories(${CMAKE_SOURCE_DIR}/Libraries/FileHandler/libpng)

add_executable(iffexport iffexport.c ${IFF_SOURCES})