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
    set(FARDIVE_LINK -mwindows ole32 uxtheme)
endif()

add_executable(FARDive ${FARDIVE_SOURCES})
target_link_libraries(FARDive ${FARDIVE_LINK} libpng_shared)