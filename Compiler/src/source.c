#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, const char* argv[]) {
    initVM();

    Chunk chunk;
    initChunk(&chunk);
    
    writeConstant(&chunk, 4.5, 122);

    uint32_t constant = addConstant(&chunk, 1.2);
    writeChunk(&chunk, OP_CONSTANT, 123);
    writeChunk(&chunk, constant, 123);
    
    writeChunk(&chunk, OP_RETURN, 123);

    //disassebleChunk(&chunk, "test chunk");
    interpret(&chunk);

    freeChunk(&chunk);
    freeVM();
    return 0;
}