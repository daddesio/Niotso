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
    set(FARDIVE_LINK ole32 uxtheme)
    add_executable(FARDive WIN32 ${FARDIVE_SOURCES})
endif()

include_directories(${CMAKE_SOURCE_DIR}/Libraries/FileHandler/libpng)
target_link_libraries(FARDive ${FARDIVE_LINK} libpng_shared)