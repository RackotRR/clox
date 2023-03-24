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
#define BINARY_OP(op) \
    do { \
        double b = pop(); \
        double a = pop(); \
        push(a op b); \
    } while(false)

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
            uint16_t constant_id;
            uint8_t* constant_id_bytes = (uint8_t*)&constant_id;
            constant_id_bytes[0] = READ_BYTE();
            constant_id_bytes[1] = READ_BYTE();

            Value constant = vm.chunk->constants.values[constant_id];
            push(constant);
            break;
        }
        case OP_ADD: BINARY_OP(+); break;
        case OP_SUBTRACT: BINARY_OP(-); break;
        case OP_MULTILPY: BINARY_OP(*); break;
        case OP_DIVIDE: BINARY_OP(/ ); break;
        case OP_NEGATE: push(-pop()); break; 
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
#undef BINARY_OP
}

InterpretResult interpret(Chunk* chunk) {
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;
    return run();
}