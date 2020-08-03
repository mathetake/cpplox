#ifndef cpplox_object_h
#define cpplox_object_h

#include "common.hpp"
#include "value.hpp"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_STRING(value) isObjType(value, OBJ_STRING)

#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->str.c_str())

enum ObjType {
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
  ObjString(const char* chars, int length);
  ObjString(std::string str);
  std::string str;
  uint32_t hash;
};

bool isObjType(Value value, ObjType type);

ObjString* allocateStringObject(const char* chars, int length);

void printObject(Value value);

uint32_t hashString(const char* key, int length);

#endif
