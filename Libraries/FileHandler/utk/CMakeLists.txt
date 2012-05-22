cmake_minimum_required(VERSION 2.6)
project(utk)

set(UTK_SOURCES
    read_utk.c
    utkdecode.c
)

add_executable(utkdecode ${UTK_SOURCES})