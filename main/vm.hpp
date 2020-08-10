#ifndef cpplox_vm_h
#define cpplox_vm_h

#include "chunk.hpp"
#include "object.hpp"
#include "table.hpp"
#include "value.hpp"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

enum IntepretResult {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR,
};

struct CallFrame {
  ObjClosure* closure;
  uint8_t* ip;
  Value* slots;
};

class VM {
 public:
  Value stack[STACK_MAX];
  Value* stack_top;
  Obj* objects;
  Table strings;
  Table globals;
  ObjUpvalue* openUpvalues;

  CallFrame frames[FRAMES_MAX];
  int frameCount;

  VM();
  ~VM();

  // set chunk
  IntepretResult run();
  IntepretResult interpret(ObjFunction* function);
  IntepretResult interpret(const char* source);
  void initVM();
  void freeVM();

  void reset_stack();
  void push(Value value);
  Value pop();
  Value peek(int distance);
  static bool isFalsey(Value value);

  bool callValue(Value callee, int argCount);
  bool call(ObjClosure* function, int argCount);

  void defineNative(const char* name, int length, NativeFunctionPtr function);

  void concatenate();
  void runtimeError(const char* format, ...);

  ObjUpvalue* captureUpvalue(Value* local);
  void closeUpvalues(Value* last);
};

void initVM();
void freeVM();

extern VM vm;
#endif
