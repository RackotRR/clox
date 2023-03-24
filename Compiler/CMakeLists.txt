cmake_minimum_required(VERSION 3.20)

project(lox_compiler)

add_executable(clox
    src/chunk.c
    src/debug.c
    src/lines_info.c
    src/memory.c
    src/source.c
    src/value.c)
target_include_directories(clox PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)