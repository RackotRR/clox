cmake_minimum_required(VERSION 3.20)

project(lox_compiler)

add_library(clox_compiler
    src/compiler.c
    src/scanner.c)
target_include_directories(clox_compiler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(clox_compiler PRIVATE clox_lib)