#ifndef clox_compiler_h
#define clox_compiler_h

#include "common/common.h"
#include "common/chunk/chunk.h"

bool compile(const char* source, Chunk* chunk);

#endif // !clox_compiler_h
