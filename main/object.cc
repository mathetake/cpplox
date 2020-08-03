#include "object.hpp"

#include "vm.hpp"

ObjString::ObjString(const char* chars, int length) {
  str = std::string(chars, length);
  type = ObjType::OBJ_STRING;
  next = nullptr;
}

ObjString::ObjString(std::string s) {
  str = s;
  type = ObjType::OBJ_STRING;
  next = nullptr;
}

bool isObjType(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
};

ObjString* allocateStringObject(const char* chars, int length) {
  auto string = new ObjString(chars, length);
  vm.objects = string;
  return string;
};

void printObject(Value value) {
  switch (OBJ_TYPE(value)) {
    case OBJ_STRING:
      printf("%s", AS_CSTRING(value));
      break;
  }
}
