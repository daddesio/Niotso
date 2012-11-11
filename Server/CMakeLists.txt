cmake_minimum_required(VERSION 2.6)
project(NiotsoServer)

if(WIN32)
else()
    set(NIOTSOSERVER_SOURCES
        Server.cpp
    )
    add_executable(niotsod-vanilla ${NIOTSOSERVER_SOURCES})
    target_link_libraries(niotsod-vanilla)
endif()
