#include "vm.hpp"

#include <iostream>

#include "common.hpp"
#include "compiler.hpp"
#include "debug.hpp"
#include "object.hpp"
#include "value.hpp"

#define DEBUG_TRACE_EXECUTION
VM vm{};

VM::VM() {
  objects = nullptr;
  reset_stack();
}

VM::~VM() { freeVM(); }

void VM::reset_stack() { stack_top = stack; }

void VM::initVM() { reset_stack(); };

void VM::freeVM() {
  auto obj = objects;
  while (obj != NULL) {
    auto next = obj->next;
    delete obj;
    obj = next;
  }
  objects = nullptr;
};

void VM::push(Value value) { *(stack_top++) = value; };

Value VM::pop() { return *(--stack_top); };

Value VM::peek(int distance) { return stack_top[-1 - distance]; }

bool VM::isFalsy(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

void VM::runtimeError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  size_t instruction = *(vm.ip - vm.chunk->code.front()) - 1;
  int line = vm.chunk->lines[instruction];
  fprintf(stderr, "[line %d] in script\n", line);
  reset_stack();
}

IntepretResult VM::run() {
#define READ_BYTE() (*ip++)
#define READ_CONSTANT() (chunk->constants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define BINARY_OP(valueType, OP)                      \
  do {                                                \
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
      runtimeError("Operands must benumbers.");       \
      return INTERPRET_RUNTIME_ERROR;                 \
    }                                                 \
    double b = AS_NUMBER(pop());                      \
    double a = AS_NUMBER(pop());                      \
    push(valueType(a OP b));                          \
  } while (false);
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
      case OP_NOT: {
        push(BOOL_VAL(isFalsy(pop())));
        break;
      }
      case OP_NEGATE: {
        if (!IS_NUMBER(peek(0))) {
          runtimeError("Operand must be a number.");
          return INTERPRET_RUNTIME_ERROR;
        }
        push(NUMBER_VAL(-AS_NUMBER(pop())));
        break;
      }
      case OP_GREATER:
        BINARY_OP(BOOL_VAL, >);
        break;
      case OP_LESS:
        BINARY_OP(BOOL_VAL, <);
        break;
      case OP_ADD: {
        if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
          concatenate();
        } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
          double b = AS_NUMBER(pop());
          double a = AS_NUMBER(pop());
          push(NUMBER_VAL(a + b));
        } else {
          runtimeError("Operands must be two numbers or two strings.");
          return INTERPRET_RUNTIME_ERROR;
        }
        break;
      }
      case OP_SUBTRACT: {
        BINARY_OP(NUMBER_VAL, -);
        break;
      }
      case OP_MULTIPLY: {
        BINARY_OP(NUMBER_VAL, *);
        break;
      }
      case OP_DIVIDE: {
        BINARY_OP(NUMBER_VAL, /);
        break;
      }
      case OP_NIL:
        push(NIL_VAL);
        break;
      case OP_TRUE:
        push(BOOL_VAL(true));
        break;
      case OP_FALSE:
        push(BOOL_VAL(false));
        break;
      case OP_PRINT:
        printValue(pop());
        printf("\n");
        break;
      case OP_POP:
        pop();
        break;
      default: {
        return IntepretResult::INTERPRET_RUNTIME_ERROR;
      }
      case OP_EQUAL: {
        Value b = pop();
        Value a = pop();
        push(BOOL_VAL(valuesEqual(a, b)));
        break;
      }
      case OP_DEFINE_GLOBAL: {
        ObjString* name = READ_STRING();
        globals.set(name, peek(0));
        pop();
        break;
      }
      case OP_RETURN: {
        return IntepretResult::INTERPRET_OK;
      }
    }
  }
#undef BINARY_OP
#undef READ_STRING
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

void VM::concatenate() {
  auto b = AS_STRING(pop());
  auto a = AS_STRING(pop());

  ObjString* ret = new ObjString{
      a->str + b->str,
  };

  push(OBJ_VAL(ret));
};
