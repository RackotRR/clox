#include <stdio.h>
#include <string.h>

#include "common/memory/memory.h"
#include "common/table/table.h"
#include "vm/vm.h"
#include "object.h"

#define ALLOCATE_OBJ(type, object_type) \
    (type*)allocateObject(sizeof(type), object_type)

static Obj* allocateObject(size_t size, ObjType type) {
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;

    object->next = vm.objects;
    vm.objects = object;
    return object;
}

// FNV-1a
static uint32_t hashString(const char* key, int length) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; ++i) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

static ObjString* allocateString(char* chars, int length, bool is_constant, uint32_t hash) {
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->is_constant = is_constant;
    string->chars = chars;
    string->hash = hash;
    tableSet(&vm.strings, string, NIL_VAL);
    return string;
}

ObjString* takeString(char* chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL) {
        FREE_ARRAY(char, chars, length + 1ull);
        return interned;
    }

    return allocateString(chars, length, false, hash);
}
ObjString* constantString(const char* chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);

    if (interned != NULL) return interned;
    return allocateString(chars, length, true, hash);
}

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
    case OBJ_STRING: {
        ObjString* string = AS_STRING(value);
        printf("%.*s", string->length, string->chars);
    } break;
    default:
        break;
    }
}