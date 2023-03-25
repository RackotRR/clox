#ifndef clox_object_h
#define clox_object_h

#include "common/common.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

typedef enum {
    OBJ_STRING
} ObjType;

#define IS_STRING(value) isObjType(value, OBJ_STRING)

#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

struct Obj {
    ObjType type;
    struct Obj* next;
};

struct ObjString {
    Obj obj;
    int length;
    bool is_constant;
    char* chars;
};

ObjString* takeString(char* chars, int length);
ObjString* constantString(const char* chars, int length);
void printObject(Value value);
bool objectsEqual(Obj* obj1, Obj* obj2);

static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif