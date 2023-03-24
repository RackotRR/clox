#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"
#include "lines_info.h"

typedef enum {
    OP_CONSTANT,
    OP_RETURN
} OpCode;

typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    LinesInfo lines_info;
    ValueArray constants;
} Chunk;

void initChunk(Chunk* chuck);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
int addConstant(Chunk* chunk, Value value);

#endif