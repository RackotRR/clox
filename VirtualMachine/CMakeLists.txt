cmake_minimum_required(VERSION 3.20)
project(loxvm)

add_library(loxvm
    src/vm.c)
target_include_directories(loxvm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(loxvm PRIVATE clox_shared)