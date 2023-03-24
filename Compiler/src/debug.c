#include <stdio.h>

#include "debug.h"

void disassebleChunk(Chunk* chunk, const char* name) {
    printf("== %s ==\n", name);
    
    for (int offset = 0; offset < chunk->count;) {
        offset = disassembleInstruction(chunk, offset);        
    }
}

static int simpleInstruction(const char* name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}
static int constantInstruction(const char* name, Chunk* chunk, int offset) {
    uint8_t constant = chunk->code[offset + 1];
    printf("%-16s %4d '", name, constant);
    printValue(chunk->constants.values[constant]);
    printf("'\n");
    return offset + 2;
}
static int longConstantInstruction(const char* name, Chunk* chunk, int offset) {
    uint8_t* constant_code = &chunk->code[offset];
    uint32_t constant = 0;
    uint8_t* constant_bytes = (uint8_t*)&constant;
    for (int i = 1; i < 4; ++i) {
        constant_bytes[i] = constant_code[i];
    }

    printf("%-16s %4d '", name, constant);
    printValue(chunk->constants.values[constant]);
    printf("'\n");
    return offset + 4;
}

int disassembleInstruction(Chunk* chunk, int offset) {
    printf("%04d ", offset);
    if (offset > 0 && 
        getLine(&chunk->lines_info, offset) == getLine(&chunk->lines_info, offset - 1)) 
    {
        printf("   | ");
    }
    else {
        printf("%04d ", getLine(&chunk->lines_info, offset));
    }

    uint8_t instruction = chunk->code[offset];
    switch (instruction) {
    case OP_CONSTANT:
        return constantInstruction("OP_CONSTANT", chunk, offset);
    case OP_CONSTANT_LONG:
        return longConstantInstruction("OP_CONSTANT_LONG", chunk, offset);
    case OP_RETURN:
        return simpleInstruction("OP_RETURN", offset);
    
    default:
        printf("Unknown opcode %d\n", instruction);
        return offset + 1;
    }
}