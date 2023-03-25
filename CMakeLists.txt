cmake_minimum_required(VERSION 3.20)
project(clox)

add_executable(clox
    src/main.c
    src/vm/vm.c
    src/debug/debug.c
    src/debug/lines_info.c
    src/compiler/compiler.c
    src/compiler/scanner.c
    src/common/chunk/chunk.c
    src/common/memory/memory.c
    src/common/value/value.c
    src/common/value/object.c
)
target_include_directories(clox PRIVATE src)