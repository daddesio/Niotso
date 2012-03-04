cmake_minimum_required(VERSION 2.6)
project(iff)

set(IFF_SOURCES
    chunks.c
    iff.c
    iffexport.c
)

add_executable(iffexport ${IFF_SOURCES})