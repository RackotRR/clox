cmake_minimum_required(VERSION 3.20)

project(lox_compiler)

add_library(clox_shared STATIC
    src/chunk.c
    src/debug.c
    src/lines_info.c
    src/memory.c
    src/value.c)
target_include_directories(clox_shared PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include)


add_executable(clox
    src/source.c)
target_link_libraries(clox PRIVATE clox_shared)
target_link_libraries(clox PRIVATE loxvm)