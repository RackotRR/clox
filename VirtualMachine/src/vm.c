#include <stdio.h>

#include "common.h"
#include "debug.h"
#include "vm.h"

static VM vm;

static void resetStack() {
    vm.stack_top = vm.stack;
}
void initVM() {
    resetStack();
}

void freeVM() {

}

void push(Value value) {
    *vm.stack_top = value;
    vm.stack_top++;
}
Value pop() {
    vm.stack_top--;
    return *vm.stack_top;
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        printf("\t");
        for (Value* slot = vm.stack; slot < vm.stack_top; ++slot) {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");
        disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif // DEBUG_TRACE_EXECUTION


        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
        case OP_CONSTANT: {
            Value constant = READ_CONSTANT();
            push(constant);
            break;
        }
        case OP_CONSTANT_LONG: {
            uint8_t constant_id_bytes[4];
            constant_id_bytes[0] = 0;
            for (int i = 1; i < 4; ++i) {
                constant_id_bytes[i] = READ_BYTE();
            }
            uint32_t* constant_id = (uint32_t*)constant_id_bytes;

            Value constant = vm.chunk->constants.values[*constant_id];
            push(constant);
            break;
        }
        case OP_RETURN: {
            printValue(pop());
            printf("\n");
            return INTERPRET_OK;
        }
        default:
            printf("default case vm run");
            break;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
}

InterpretResult interpret(Chunk* chunk) {
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;
    return run();
}