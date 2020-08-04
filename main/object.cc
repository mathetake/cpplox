#include "object.hpp"

#include "vm.hpp"

ObjString::ObjString(const char* chars, int length) {
  str = std::string(chars, length);
  type = ObjType::OBJ_STRING;
  next = nullptr;
  hash = hashString(chars, length);
}

ObjString::ObjString(std::string s) {
  str = s;
  type = ObjType::OBJ_STRING;
  next = nullptr;
  hash = hashString(s.c_str(), s.size());
}

bool isObjType(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
};

ObjString* allocateStringObject(const char* chars, int length) {
  auto string = new ObjString(chars, length);
  if (auto found = vm.strings.findString(string); found != nullptr)
    return found;
  vm.objects = string;
  vm.strings.set(string, NIL_VAL);
  return string;
};

void printObject(Value value) {
  switch (OBJ_TYPE(value)) {
    case OBJ_STRING:
      printf("%s", AS_CSTRING(value));
      break;
  }
}

uint32_t hashString(const char* key, int length) {
  uint32_t hash = 2166136261u;

  for (int i = 0; i < length; i++) {
    hash ^= key[i];
    hash *= 16777619;
  }

  return hash;
}
