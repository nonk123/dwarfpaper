cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

project(dwarfpaper
    VERSION 0.2.0
    DESCRIPTION "A Dwarf Fortress styled live wallpaper app"
    LANGUAGES C)

set(BUILD_SHARED_LIBS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(noise
    GIT_REPOSITORY https://github.com/smcameron/open-simplex-noise-in-c.git
    GIT_TAG 53cf5bfd4a5289f263568bedd778e2d86eb8a1ef)

FetchContent_Declare(stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG f58f558c120e9b32c217290b80bad1a0729fbb2c)

FetchContent_Declare(SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.14)

set(SDL_SHARED OFF)
set(SDL_STATIC ON)
set(SDL_TEST_LIBRARY OFF)
set(SDL_GPU OFF)
set(SDL_AUDIO OFF)
set(SDL_CAMERA OFF)
set(SDL_JOYSTICK OFF)
set(SDL_HAPTIC OFF)
set(SDL_POWER OFF)
set(SDL_SENSOR OFF)
set(SDL_DIALOG OFF)
set(SDL_TRAY OFF)

FetchContent_MakeAvailable(noise stb SDL3)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
file(GLOB_RECURSE SOURCES ${SRC_DIR}/*.c)
file(GLOB_RECURSE HEADERS ${SRC_DIR}/*.h)

set(SDL_SHARED FALSE)
set(SDL_STATIC TRUE)

add_library(noise ${noise_SOURCE_DIR}/open-simplex-noise.c
                  ${noise_SOURCE_DIR}/open-simplex-noise.h)
target_include_directories(noise PUBLIC ${noise_SOURCE_DIR})

add_executable(dwarfpaper ${SOURCES} ${HEADERS} ${SRC_DIR}/vga9x16.rc)
target_link_libraries(dwarfpaper PRIVATE SDL3-static noise)
target_include_directories(dwarfpaper PRIVATE ${SRC_DIR} ${stb_SOURCE_DIR})
target_compile_definitions(dwarfpaper PRIVATE _CRT_SECURE_NO_WARNINGS=1)
#target_link_options(dwarfpaper PRIVATE $<IF:$<BOOL:${MSVC}>,/SUBSYSTEM:WINDOWS,-mwindows>)
target_link_libraries(dwarfpaper PRIVATE user32.lib)
