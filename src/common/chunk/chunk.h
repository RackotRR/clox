#ifndef clox_chunk_h
#define clox_chunk_h

#include "common/common.h"
#include "common/value/value.h"
#include "debug/lines_info.h"

typedef enum {
    OP_CONSTANT,
    OP_CONSTANT_LONG,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTILPY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
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
void writeConstant(Chunk* chunk, Value value, int line);
uint16_t addConstant(Chunk* chunk, Value value);


#endif