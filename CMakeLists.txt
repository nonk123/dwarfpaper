cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

project(
    dwarfpaper
    VERSION 0.1.0
    DESCRIPTION "A Dwarf Fortress styled live wallpaper app"
    LANGUAGES C)

include(FetchContent)

FetchContent_Declare(
    noise
    GIT_REPOSITORY "https://github.com/smcameron/open-simplex-noise-in-c.git"
    GIT_TAG 53cf5bfd4a5289f263568bedd778e2d86eb8a1ef)
FetchContent_MakeAvailable(noise)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.c")
file(GLOB_RECURSE HEADERS "${SRC_DIR}/*.h")

add_library(noise ${noise_SOURCE_DIR}/open-simplex-noise.c
                  ${noise_SOURCE_DIR}/open-simplex-noise.h)
target_include_directories(noise PUBLIC ${noise_SOURCE_DIR})

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE noise)
target_include_directories(${PROJECT_NAME} PRIVATE ${SRC_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS=1)
