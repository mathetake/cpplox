#include "vm.hpp"

#include <iostream>

#include "common.hpp"
#include "compiler.hpp"
#include "debug.hpp"

#define DEBUG_TRACE_EXECUTION

VM vm{};

VM::VM() { reset_stack(); }

void VM::reset_stack() { stack_top = stack; }

void VM::initVM() { reset_stack(); };

void VM::push(Value value) { *(stack_top++) = value; };

Value VM::pop() { return *(--stack_top); };

IntepretResult VM::run() {
#define READ_BYTE() (*ip++)
#define READ_CONSTANT() (chunk->constants.values[READ_BYTE()])
#define BINARY_OP(OP) \
  do {                \
    double b = pop(); \
    double a = pop(); \
    push(a OP b);     \
  } while (false)

  uint8_t inst;
  while (true) {
#ifdef DEBUG_TRACE_EXECUTION
    printf("          ");
    for (Value* slot = stack; slot < stack_top; ++slot) {
      printf("[");
      printValue(*slot);
      printf("]");
    }
    printf("\n");
    disassembleInstruction(chunk, static_cast<int>(ip - &chunk->code.front()));
#endif
    switch (inst = READ_BYTE()) {
      case OP_CONSTANT: {
        Value constant = READ_CONSTANT();
        push(constant);
        break;
      }
      case OP_NEGATE: {
        push(-pop());
        break;
      }
      case OP_ADD: {
        BINARY_OP(+);
        break;
      }
      case OP_SUBTRACT: {
        BINARY_OP(-);
        break;
      }
      case OP_MULTIPLY: {
        BINARY_OP(*);
        break;
      }
      case OP_DIVIDE: {
        BINARY_OP(/);
        break;
      }
      case OP_RETURN: {
        return IntepretResult::INTERPRET_OK;
      }
      default: {
        return IntepretResult::INTERPRET_RUNTIME_ERROR;
      }
    }
  }
#undef BINARY_OP
#undef READ_CONSTANT
#undef READ_BYTE
};

IntepretResult VM::interpret(Chunk* c) {
  chunk = c;
  ip = &(chunk->code.front());
  return IntepretResult::INTERPRET_OK;
}

IntepretResult VM::interpret(const char* source) {
  std::cout << source << "\n";

  auto chunk = Chunk{};
  auto compiler = Compiler(source, &chunk);
  if (!compiler.compile()) {
    return IntepretResult::INTERPRET_COMPILE_ERROR;
  }

  interpret(&chunk);
  return run();
}
