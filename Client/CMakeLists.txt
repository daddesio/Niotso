cmake_minimum_required(VERSION 2.6)
project(NiotsoClient)

include_directories(${FILEHANDLER_INCLUDE} ${FREETYPE_INCLUDE})

if(WIN32)
    set(NIOTSOCLIENT_SOURCES
        Client.cpp
        Audio/Startup.cpp
        Audio/windows/XAudio2.cpp
        Graphics/Font.cpp
        Graphics/Startup.cpp
        Graphics/Viewport.cpp
        resources/Resource.rc
        Scene/LoginScreen/LoginScreen.cpp
        System/System.cpp
        Window/Window.cpp
    )
    add_executable(TSO WIN32 ${NIOTSOCLIENT_SOURCES})
    target_link_libraries(TSO ole32 opengl32)
    target_link_libraries(TSO FileHandler_shared ${FREETYPE_LINK})
endif()