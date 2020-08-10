#include "object.hpp"

#include "value.hpp"
#include "vm.hpp"

ObjString::ObjString(const char* chars, int length) {
  str = std::string(chars, length);
  type = ObjType::OBJ_STRING;
  next = nullptr;
  hash = hashString(chars, length);
}

ObjString::ObjString(std::string s) : str(s) {
  type = ObjType::OBJ_STRING;
  next = nullptr;
  hash = hashString(s.c_str(), s.size());
}

bool isObjType(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
};

#define ADD_OBJECT_LISTS(list, target) \
  {                                    \
    target->next = *list;              \
    *list = target;                    \
  }

ObjString* allocateStringObject(const char* chars, int length,
                                Table* stringTable, Obj** objects) {
  auto string = new ObjString(chars, length);
  string->type = ObjType::OBJ_STRING;
  string->isMarked = false;
  auto found = stringTable->findString(string);
  if (found != nullptr) return found;

  ADD_OBJECT_LISTS(objects, string)
  stringTable->set(string, NIL_VAL);
  return string;
};

ObjFunction* allocateFunctionObject(Obj** objects) {
  auto function = new ObjFunction{};
  function->isMarked = false;
  function->type = ObjType::OBJ_FUNCTION;
  ADD_OBJECT_LISTS(objects, function)
  return function;
}

ObjNative* allocateNativeFnctionObject(NativeFunctionPtr func, Obj** objects) {
  auto native = new ObjNative{func};
  native->isMarked = false;
  native->type = ObjType::OBJ_NATIVE;
  ADD_OBJECT_LISTS(objects, native)
  return native;
}

ObjClosure* allocateClosureObject(ObjFunction* function, Obj** objects) {
  auto closure = new ObjClosure(function);
  closure->isMarked = false;
  closure->type = ObjType::OBJ_CLOSURE;
  ADD_OBJECT_LISTS(objects, closure)
  return closure;
}

ObjUpvalue* allocateUpvalueObject(Value* location, Obj** objects) {
  auto upvalue = new ObjUpvalue(location);
  upvalue->isMarked = false;
  upvalue->type = ObjType::OBJ_UPVALUE;
  ADD_OBJECT_LISTS(objects, upvalue)
  return upvalue;
};

void printFunction(ObjFunction* function) {
  if (function->name == NULL) {
    printf("<script>");
    return;
  }
  printf("<fn %s>", function->name->str.c_str());
}

void printObject(Value value) {
  switch (OBJ_TYPE(value)) {
    case OBJ_FUNCTION:
      printFunction(AS_FUNCTION(value));
      break;
    case OBJ_NATIVE:
      printf("<native fn>");
      break;
    case OBJ_CLOSURE:
      printFunction(AS_CLOSURE(value)->function);
      break;
    case OBJ_STRING:
      printf("%s", AS_CSTRING(value));
      break;
    case OBJ_UPVALUE:
      printf("upvalue");
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

#define MARK_VALUE(value) \
  if (IS_OBJ(value)) markObject(AS_OBJ(value), grayStack)

void markObject(Obj* obj, std::vector<Obj*>& grayStack) {
  if (obj == nullptr) return;
  if (obj->isMarked) return;

#ifdef DEBUG_LOG_GC
  printf("%p mark ", (void*)obj);
  printValue(OBJ_VAL(obj));
  printf("\n");
#endif
  obj->isMarked = true;

  grayStack.push_back(obj);
}

void blackenObject(Obj* obj, std::vector<Obj*>& grayStack) {
#ifdef DEBUG_LOG_GC
  printf("%p blacken ", (void*)obj);
  printValue(OBJ_VAL(obj));
  printf("\n");
#endif

  switch (obj->type) {
    case OBJ_UPVALUE:
      MARK_VALUE(((ObjUpvalue*)obj)->closed);
      break;
    case OBJ_FUNCTION: {
      ObjFunction* function = (ObjFunction*)obj;
      markObject((Obj*)function->name, grayStack);
      for (auto value : function->chunk.constants.values) {
        MARK_VALUE(value);
      }
      break;
    }
    case OBJ_CLOSURE: {
      ObjClosure* closure = (ObjClosure*)obj;
      markObject((Obj*)closure->function, grayStack);
      for (int i = 0; i < closure->upvalueCount; i++) {
        markObject((Obj*)closure->upvalues[i], grayStack);
      }
      break;
    }
    case OBJ_NATIVE:
    case OBJ_STRING:
      break;
  }
}
