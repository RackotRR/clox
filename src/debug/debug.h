#ifndef clox_debug_h
#define clox_debug_h

#include "common/chunk/chunk.h"

void disassebleChunk(Chunk* chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset);

#endif