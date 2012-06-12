cmake_minimum_required(VERSION 2.6)
project(hitutils)

set(HITDUMP_SOURCES
    hitdump.cpp
)

set(HITASM_SOURCES
    hitasm.cpp
)

set(HITLD_SOURCES
    hitld.cpp
)

add_executable(hitdump ${HITDUMP_SOURCES})
add_executable(hitasm  ${HITASM_SOURCES})
add_executable(hitld   ${HITLD_SOURCES})