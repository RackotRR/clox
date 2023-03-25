cmake_minimum_required(VERSION 3.20)
project(clox_vm)

add_library(clox_vm
    src/vm.c)
target_include_directories(clox_vm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(clox_vm PUBLIC clox_lib)
target_link_libraries(clox_vm PRIVATE clox_compiler)

add_executable(clox
    src/main.c)
target_link_libraries(clox PRIVATE clox_vm)