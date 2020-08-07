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
  frameCount = 0;
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

bool VM::isFalsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

void VM::runtimeError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  CallFrame* frame = &vm.frames[frameCount - 1];

  size_t instruction = *(frame->ip - frame->function->chunk.code.front()) - 1;
  int line = frame->function->chunk.lines[instruction];
  fprintf(stderr, "[line %d] in script\n", line);
  reset_stack();
}

IntepretResult VM::run() {
  CallFrame* frame = &frames[frameCount - 1];
#define READ_BYTE() (*frame->ip++)
#define READ_CONSTANT() (frame->function->chunk.constants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define READ_SHORT() \
  (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
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
    disassembleInstruction(
        &frame->function->chunk,
        static_cast<int>(frame->ip - &frame->function->chunk.code.front()));
#endif
    switch (inst = READ_BYTE()) {
      case OP_CONSTANT: {
        Value constant = READ_CONSTANT();
        push(constant);
        break;
      }
      case OP_NOT: {
        push(BOOL_VAL(isFalsey(pop())));
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
      case OP_GET_GLOBAL: {
        ObjString* name = READ_STRING();
        Value value;
        if (!globals.get(name, &value)) {
          runtimeError("Undefined variable '%s'.", name->str.c_str());
          return INTERPRET_RUNTIME_ERROR;
        }
        push(value);
        break;
      }
      case OP_GET_LOCAL: {
        uint8_t slot = READ_BYTE();
        push(frame->slots[slot]);
        break;
      }
      case OP_SET_GLOBAL: {
        ObjString* name = READ_STRING();
        if (globals.set(name, peek(0))) {
          globals.deleteKey(name);
          runtimeError("Undefined variable '%s'.", name->str.c_str());
          return INTERPRET_RUNTIME_ERROR;
        }
        break;
      }
      case OP_SET_LOCAL: {
        uint8_t slot = READ_BYTE();
        frame->slots[slot] = peek(0);
        break;
      }
      case OP_RETURN: {
        return IntepretResult::INTERPRET_OK;
      }
      case OP_JUMP_IF_FALSE: {
        uint16_t offset = READ_SHORT();
        if (isFalsey(peek(0))) frame->ip += offset;
        break;
      }
      case OP_JUMP: {
        uint16_t offset = READ_SHORT();
        frame->ip += offset;
        break;
      }
      case OP_LOOP: {
        uint16_t offset = READ_SHORT();
        frame->ip -= offset;
        break;
      }
    }
  }
#undef BINARY_OP
#undef READ_SHORT
#undef READ_STRING
#undef READ_CONSTANT
#undef READ_BYTE
};

IntepretResult VM::interpret(ObjFunction* function) {  // for testing purpose
  CallFrame* frame = &frames[frameCount++];
  frame->function = function;
  frame->ip = &function->chunk.code.front();
  frame->slots = stack;
  return IntepretResult::INTERPRET_OK;
}

IntepretResult VM::interpret(const char* source) {
  std::cout << source << "\n";

  auto compiler = Compiler(source, &strings, &objects);
  auto function = compiler.compile();
  if (function == nullptr) return IntepretResult::INTERPRET_COMPILE_ERROR;

  push(OBJ_VAL(function));
  CallFrame* frame = &frames[frameCount++];
  frame->function = function;
  frame->ip = &function->chunk.code.front();
  frame->slots = stack;
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
