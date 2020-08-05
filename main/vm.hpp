#ifndef cpplox_vm_h
#define cpplox_vm_h

#include "chunk.hpp"
#include "table.hpp"
#include "value.hpp"

#define STACK_MAX 256

enum IntepretResult {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR,
};

class VM {
 public:
  Chunk* chunk;
  uint8_t* ip;
  Value stack[STACK_MAX];
  Value* stack_top;
  Obj* objects;
  Table strings;
  Table globals;

  VM();
  ~VM();

  // set chunk
  IntepretResult run();
  IntepretResult interpret(Chunk* chunk);
  IntepretResult interpret(const char* source);
  void initVM();
  void freeVM();

  void reset_stack();
  void push(Value value);
  Value pop();
  Value peek(int distance);
  static bool isFalsey(Value value);

  void concatenate();
  void runtimeError(const char* format, ...);
};

void initVM();
void freeVM();

extern VM vm;
#endif
