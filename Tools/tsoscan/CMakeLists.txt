cmake_minimum_required(VERSION 2.6)
project(tsoscan)

set(TSOSCAN_SOURCES
    tsoscan.c
    cmd.c
    stats.c
)

include_directories(${FILEHANDLER_INCLUDE})

add_executable(tsoscan ${TSOSCAN_SOURCES})
target_link_libraries(tsoscan iff_static)