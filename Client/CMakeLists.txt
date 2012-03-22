cmake_minimum_required(VERSION 2.6)
project(NiotsoClient)

if(WIN32)
    set(NIOTSOCLIENT_SOURCES
        Client.cpp
        MessageHandler.cpp
        Audio/Startup.cpp
        Audio/windows/XAudio2.cpp
        Graphics/Startup.cpp
        Graphics/Viewport.cpp
        resources/Resource.rc
        System/System.cpp
    )
    add_executable(TSO WIN32 ${NIOTSOCLIENT_SOURCES})
    target_link_libraries(TSO ole32 opengl32)
endif()