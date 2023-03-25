cmake_minimum_required(VERSION 3.20)

project(clox_lib)

add_library(clox_lib
    src/chunk.c
    src/debug.c
    src/lines_info.c
    src/memory.c
    src/value.c)
target_include_directories(clox_lib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include)