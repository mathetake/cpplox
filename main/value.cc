#include "value.hpp"

void ValueArray::writeValueArray(Value value) { values.push_back(value); }

void printValue(Value value) { printf("%g", value); }

Value* ValueArray::peek() {
  if (values.empty()) return nullptr;

  return &values[values.size() - 1];
}
