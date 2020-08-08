#ifndef cpplox_object_h
#define cpplox_object_h

#include "chunk.hpp"
#include "common.hpp"
#include "table.hpp"
#include "value.hpp"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_STRING(value) isObjType(value, OBJ_STRING)
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)

#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative*)AS_OBJ(value))->func)
#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->str.c_str())

class Table;

enum ObjType {
  OBJ_FUNCTION,
  OBJ_STRING,
  OBJ_NATIVE,
};

class Obj {
 public:
  ObjType type;
  Obj* next;
  virtual ~Obj(){};
};

class ObjString : public Obj {
 public:
  ObjString(){};
  ObjString(const char* chars, int length);
  ObjString(std::string str);
  std::string str;
  uint32_t hash;
};

class ObjFunction : public Obj {
 public:
  int arity;
  Chunk chunk;
  ObjString* name;

  ObjFunction() : arity(0), name(nullptr){};
  ObjFunction(Chunk chunk) : arity(0), chunk(chunk), name(nullptr) {}
  ~ObjFunction() {
    if (name != nullptr) delete name;
  };
};

using NativeFunctionType = Value(int argCount, Value* args);
using NativeFunctionPtr = NativeFunctionType*;

class ObjNative : public Obj {
 public:
  NativeFunctionPtr func;
  ObjNative(NativeFunctionPtr func) : func(func){};
};

bool isObjType(Value value, ObjType type);

ObjString* allocateStringObject(const char* chars, int length,
                                Table* stringTable, Obj** objects);
ObjFunction* allocateFunctionObject(Obj** objects);
ObjNative* allocateNativeFnctionObject(NativeFunctionPtr func, Obj** objects);

void printObject(Value value);

uint32_t hashString(const char* key, int length);

#endif
