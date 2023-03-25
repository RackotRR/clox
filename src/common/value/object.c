#include <stdio.h>
#include <string.h>

#include "common/memory/memory.h"
#include "vm/vm.h"
#include "object.h"
#include "value.h"

#define ALLOCATE_OBJ(type, object_type) \
    (type*)allocateObject(sizeof(type), object_type)

static Obj* allocateObject(size_t size, ObjType type) {
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;

    object->next = vm.objects;
    vm.objects = object;
    return object;
}

static ObjString* allocateString(char* chars, int length, bool is_constant) {
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->is_constant = is_constant;
    string->chars = chars;
    return string;
}

ObjString* takeString(char* chars, int length) {
    return allocateString(chars, length, false);
}
ObjString* constantString(const char* chars, int length) {
    return allocateString(chars, length, true);
}

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
    case OBJ_STRING:
        printf("%s", AS_CSTRING(value));
        break;
    default:
        break;
    }
}

bool objectsEqual(Obj* a, Obj* b) {
    if (a->type != b->type) return false;

    // if (obj1->type == OBJ_STRING)
    ObjString* a_string = (ObjString*)a;
    ObjString* b_string = (ObjString*)b;
    return a_string->length == b_string->length &&
        memcmp(a_string->chars, b_string->chars, a_string->length) == 0;
}