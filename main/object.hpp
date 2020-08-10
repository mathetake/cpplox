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
#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)

#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative*)AS_OBJ(value))->func)
#define AS_CLOSURE(value) (((ObjClosure*)AS_OBJ(value)))
#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->str.c_str())

class Table;

enum ObjType {
  OBJ_FUNCTION,
  OBJ_STRING,
  OBJ_NATIVE,
  OBJ_UPVALUE,
  OBJ_CLOSURE,
};

class Obj {
 public:
  ObjType type;
  bool isMarked;
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
  int upvalueCount;

  Chunk chunk;
  ObjString* name;

  ObjFunction() : arity(0), upvalueCount(0), name(nullptr){};
  ObjFunction(Chunk chunk)
      : arity(0), upvalueCount(0), chunk(chunk), name(nullptr) {}
  ~ObjFunction() { delete name; };
};

using NativeFunctionType = Value(int argCount, Value* args);
using NativeFunctionPtr = NativeFunctionType*;

class ObjNative : public Obj {
 public:
  NativeFunctionPtr func;
  ObjNative(NativeFunctionPtr func) : func(func){};
  ~ObjNative(){};
};

class ObjUpvalue : public Obj {
 public:
  Value* location;
  Value closed;
  ObjUpvalue* nextUpValue;
  ObjUpvalue(Value* location)
      : location(location), nextUpValue(nullptr), closed(NIL_VAL){};
};

class ObjClosure : public Obj {
 public:
  ObjFunction* function;
  std::vector<ObjUpvalue*> upvalues;
  int upvalueCount;
  ObjClosure(ObjFunction* function)
      : function(function),
        upvalueCount(function->upvalueCount),
        upvalues(std::vector<ObjUpvalue*>(function->upvalueCount, NULL)){};
  ~ObjClosure(){};
};

ObjString* allocateStringObject(const char* chars, int length,
                                Table* stringTable, Obj** objects);
ObjFunction* allocateFunctionObject(Obj** objects);
ObjNative* allocateNativeFnctionObject(NativeFunctionPtr func, Obj** objects);
ObjClosure* allocateClosureObject(ObjFunction* function, Obj** objects);
ObjUpvalue* allocateUpvalueObject(Value* location, Obj** objects);

bool isObjType(Value value, ObjType type);
void printObject(Value value);
uint32_t hashString(const char* key, int length);

void markObject(Obj* obj, std::vector<Obj*>& greyStack);
void blackenObject(Obj* obj, std::vector<Obj*>& greyStack);
#endif
