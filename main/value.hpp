#ifndef cpplox_value_h
#define cpplox_value_h

#include "common.hpp"

using Value = double;

void printValue(Value value);

class ValueArray {
 public:
  std::vector<Value> values;
  ValueArray() { values = {}; };
  ~ValueArray(){};

  void writeValueArray(Value value);
  Value* peek();
};

#endif
