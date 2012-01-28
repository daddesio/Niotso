cmake_minimum_required(VERSION 2.6)
project(xa)

set(XA_SOURCES
    read_xa.c
    xadecode.c
)

add_executable(xadecode ${XA_SOURCES})