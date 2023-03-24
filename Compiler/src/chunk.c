#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

void initChunk(Chunk* chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    initLinesInfo(&chunk->lines_info);
    initValueArray(&chunk->constants);
}

void freeChunk(Chunk* chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    freeLinesInfo(&chunk->lines_info);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}

void writeChunk(Chunk* chunk, uint8_t byte, int line) {
    if (chunk->capacity < chunk->count + 1) {
        int old_capacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(old_capacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, 
            old_capacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->count++;

    writeLinesInfo(&chunk->lines_info, line);
}

void writeConstant(Chunk* chunk, Value value, int line) {
    writeChunk(chunk, OP_CONSTANT_LONG, line);

    uint16_t constant = addConstant(chunk, value);
    uint8_t* bytes = (uint8_t*)&constant;

    writeChunk(chunk, bytes[0], line);
    writeChunk(chunk, bytes[1], line);
}

uint16_t addConstant(Chunk* chunk, Value value) {
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}
