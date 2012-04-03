cmake_minimum_required(VERSION 2.6)
project(iff)

set(IFF_SOURCES
	bhav.c
	stbl.c
	chunks.c
    iff.c
)

add_executable(iffexport iffexport.c ${IFF_SOURCES})