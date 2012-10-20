cmake_minimum_required(VERSION 2.6)
project(FARDive)

if(WIN32)
    set(FARDIVE_SOURCES
        windows/common.cpp
        windows/Interaction.cpp
        windows/MainWindow.cpp
        windows/ReadPNGIcon.cpp
        windows/Resource.rc
        windows/Startup.cpp
        windows/Dialog/AddToArchive.cpp
        windows/Dialog/NewArchive.cpp
    )
    include_directories(${LIBPNG_INCLUDE})
    add_executable(FARDive WIN32 ${FARDIVE_SOURCES})
    target_link_libraries(FARDive ole32 uxtheme FileHandler_shared)
endif()