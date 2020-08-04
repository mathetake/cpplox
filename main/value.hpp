#ifndef cpplox_value_h
#define cpplox_value_h

#include "common.hpp"

struct Obj;
struct ObjString;

enum ValueType {
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER,
  VAL_OBJ,
};

struct Value {
  ValueType type;
  union {
    bool boolean;
    double number;
    Obj* obj;
  };
};

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_OBJ(value) ((value).type == VAL_OBJ)

#define AS_OBJ(value) ((value).obj)
#define AS_BOOL(value) ((value).boolean)
#define AS_NUMBER(value) ((value).number)

#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})
#define OBJ_VAL(object) ((Value){VAL_OBJ, {.obj = object}})

void printValue(Value value);

bool valuesEqual(Value a, Value b);

class ValueArray {
 public:
  std::vector<Value> values;
  ValueArray() { values = {}; };
  ~ValueArray(){};

  void writeValueArray(Value value);
  Value* peek();
};

#endif
