#include "value.hpp"

void ValueArray::writeValueArray(Value value) { values.push_back(value); }

void printValue(Value value) {
  switch (value.type) {
    case ValueType::VAL_BOOL:
      printf(AS_BOOL(value) ? "true" : "false");
      break;
    case ValueType::VAL_NIL:
      printf("nil");
      break;
    case ValueType::VAL_NUMBER:
      printf("%g", AS_NUMBER(value));
      break;
    default:
      break;
  }
}

Value* ValueArray::peek() {
  if (values.empty()) return nullptr;

  return &values[values.size() - 1];
}

bool valuesEqual(Value a, Value b) {
  if (a.type != b.type) return false;

  switch (a.type) {
    case ValueType::VAL_BOOL:
      return AS_BOOL(a) == AS_BOOL(b);
    case ValueType::VAL_NUMBER:
      return AS_NUMBER(a) == AS_NUMBER(b);
    case ValueType::VAL_NIL:
      return true;
  }
}
