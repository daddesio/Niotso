cmake_minimum_required(VERSION 2.6)
project(libvitaboy)

set(LIBVITABOY_SERIES 0)
set(LIBVITABOY_MAJOR 0)
set(LIBVITABOY_MINOR 1)

set(LIBVITABOY_SOURCES
    anim.cpp
    libvitaboy.cpp
    mesh.cpp
    skel.cpp
)
if(WIN32)
    set(LIBVITABOY_SOURCES ${LIBVITABOY_SOURCES} resource.rc)
endif()

add_library(libvitaboy_static STATIC ${LIBVITABOY_SOURCES})
set_target_properties(libvitaboy_static PROPERTIES
    OUTPUT_NAME "vitaboy"
    CLEAN_DIRECT_OUTPUT 1)

add_library(libvitaboy_shared SHARED ${LIBVITABOY_SOURCES})
set_target_properties(libvitaboy_shared PROPERTIES
    OUTPUT_NAME "vitaboy${LIBVITABOY_SERIES}"
    VERSION ${LIBVITABOY_SERIES}.${LIBVITABOY_MAJOR}.${LIBVITABOY_MINOR}
    SOVERSION ${LIBVITABOY_SERIES}
    # msvc does not prepend 'lib' - do it here to have consistent name
        PREFIX "lib"
        IMPORT_PREFIX "lib"
    CLEAN_DIRECT_OUTPUT 1)

add_executable(vbparse vbparse.cpp)
target_link_libraries(vbparse libvitaboy_static)

add_executable(Renderer Renderer.cpp)
target_link_libraries(Renderer libvitaboy_shared "${CMAKE_SOURCE_DIR}/Libraries/libvitaboy/libSOIL.a" opengl32 glu32 winmm)