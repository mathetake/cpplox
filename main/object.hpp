#ifndef cpplox_object_h
#define cpplox_object_h

#include "chunk.hpp"
#include "common.hpp"
#include "table.hpp"
#include "value.hpp"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_STRING(value) isObjType(value, OBJ_STRING)

#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))
#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->str.c_str())

class Table;

enum ObjType {
  OBJ_FUNCTION,
  OBJ_STRING,
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

  ObjFunction() : arity(0), name(new ObjString{}){};
  ObjFunction(Chunk chunk) : arity(0), chunk(chunk), name(new ObjString{}) {}
  ~ObjFunction() { delete name; };
};

bool isObjType(Value value, ObjType type);

ObjString* allocateStringObject(const char* chars, int length,
                                Table* stringTable, Obj** objects);

void printObject(Value value);

uint32_t hashString(const char* key, int length);

#endif
