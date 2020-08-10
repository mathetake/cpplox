#include "vm.hpp"

#include <time.h>

#include <iostream>

#include "common.hpp"
#include "compiler.hpp"
#include "debug.hpp"
#include "object.hpp"
#include "value.hpp"

#define DEBUG_TRACE_EXECUTION

Value clockNative(int argCount, Value* args) {
  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

VM vm{};

VM::VM() : objects(nullptr), frameCount(0), openUpvalues(nullptr) {
  reset_stack();
  defineNative("clock", 5, clockNative);
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

  CallFrame* frame = &frames[frameCount - 1];

  size_t instruction =
      *(frame->ip - frame->closure->function->chunk.code.front()) - 1;
  int line = frame->closure->function->chunk.lines[instruction];
  fprintf(stderr, "[line %d] in script\n", line);

  for (int i = frameCount - 1; i >= 0; i--) {
    CallFrame* frame = &frames[i];
    ObjFunction* function = frame->closure->function;
    size_t instruction = *(frame->ip - function->chunk.code.front() - 1);
    fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);
    if (function->name == NULL) {
      fprintf(stderr, "script\n");
    } else {
      fprintf(stderr, "%s()\n", function->name->str.c_str());
    }
  }

  reset_stack();
}

void VM::defineNative(const char* name, int length,
                      NativeFunctionPtr function) {
  push(OBJ_VAL(allocateStringObject(name, length, &strings, &objects)));
  push(OBJ_VAL(allocateNativeFnctionObject(function, &objects)));
  globals.set(AS_STRING(stack[0]), stack[1]);
  pop();
  pop();
}

IntepretResult VM::run() {
  CallFrame* frame = &frames[frameCount - 1];
#define READ_BYTE() (*frame->ip++)
#define READ_CONSTANT() \
  (frame->closure->function->chunk.constants.values[READ_BYTE()])
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
        &frame->closure->function->chunk,
        static_cast<int>(frame->ip -
                         &frame->closure->function->chunk.code.front()));
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
        Value result = pop();

        closeUpvalues(frame->slots);

        frameCount--;
        if (frameCount == 0) {
          pop();
          return INTERPRET_OK;
        }

        stack_top = frame->slots;
        push(result);

        frame = &frames[frameCount - 1];
        break;
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
      case OP_CALL: {
        int argCount = READ_BYTE();
        if (!callValue(peek(argCount), argCount)) {
          return INTERPRET_RUNTIME_ERROR;
        }
        frame = &frames[frameCount - 1];  // switch to new function frame
        break;
      }
      case OP_CLOSURE: {
        ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
        ObjClosure* closure = allocateClosureObject(function, &objects);
        push(OBJ_VAL(closure));
        for (int i = 0; i < closure->upvalueCount; i++) {
          uint8_t isLocal = READ_BYTE();
          uint8_t index = READ_BYTE();
          if (isLocal) {
            closure->upvalues[i] = captureUpvalue(frame->slots + index);
          } else {
            closure->upvalues[i] = frame->closure->upvalues[index];
          }
        }
        break;
      }
      case OP_GET_UPVALUE: {
        uint8_t slot = READ_BYTE();
        push(*frame->closure->upvalues[slot]->location);
        break;
      }
      case OP_SET_UPVALUE: {
        uint8_t slot = READ_BYTE();
        *frame->closure->upvalues[slot]->location = peek(0);
        break;
      }
      case OP_CLOSE_UPVALUE:
        closeUpvalues(stack_top - 1);
        pop();
        break;
    }
  }
#undef BINARY_OP
#undef READ_SHORT
#undef READ_STRING
#undef READ_CONSTANT
#undef READ_BYTE
};

IntepretResult VM::interpret(ObjFunction* function) {  // for testing purpose
  ObjClosure* closure = allocateClosureObject(function, &objects);
  push(OBJ_VAL(closure));
  callValue(OBJ_VAL(closure), 0);
  return IntepretResult::INTERPRET_OK;
}

IntepretResult VM::interpret(const char* source) {
  std::cout << source << "\n";

  auto compiler = Compiler(source, &strings, &objects);
  auto function = compiler.compile();
  if (function == nullptr) return IntepretResult::INTERPRET_COMPILE_ERROR;

  push(OBJ_VAL(function));
  ObjClosure* closure = allocateClosureObject(function, &objects);
  pop();
  push(OBJ_VAL(closure));
  callValue(OBJ_VAL(closure), 0);
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

bool VM::callValue(Value callee, int argCount) {
  if (IS_OBJ(callee)) {
    switch (OBJ_TYPE(callee)) {
      case OBJ_CLOSURE:
        return call(AS_CLOSURE(callee), argCount);
      case OBJ_NATIVE: {
        NativeFunctionPtr native = AS_NATIVE(callee);
        Value result = native(argCount, stack_top - argCount);
        stack_top -= argCount + 1;
        push(result);
        return true;
      }
      default:
        // Non-callable object type.
        break;
    }
  }
  runtimeError("Can only call functions and classes.");
  return false;
};

bool VM::call(ObjClosure* closure, int argCount) {
  if (argCount != closure->function->arity) {
    runtimeError("Expected %d arguments but got %d.", closure->function->arity,
                 argCount);
    return false;
  }

  if (frameCount == FRAMES_MAX) {
    runtimeError("Stack overflow.");
    return false;
  }

  CallFrame* frame = &frames[frameCount++];
  frame->closure = closure;
  frame->ip = &closure->function->chunk.code.front();
  frame->slots = stack_top - argCount - 1;
  return true;
};

ObjUpvalue* VM::captureUpvalue(Value* local) {
  ObjUpvalue* prevUpvalue = NULL;
  ObjUpvalue* upvalue = openUpvalues;

  while (upvalue != NULL && upvalue->location > local) {
    prevUpvalue = upvalue;
    upvalue = upvalue->nextUpValue;
  }

  if (upvalue != NULL && upvalue->location == local) return upvalue;
  ObjUpvalue* createdUpvalue = allocateUpvalueObject(local, &objects);

  createdUpvalue->next = upvalue;

  if (prevUpvalue == NULL) {
    openUpvalues = createdUpvalue;
  } else {
    prevUpvalue->next = createdUpvalue;
  }
  return createdUpvalue;
}

void VM::closeUpvalues(Value* last) {
  while (openUpvalues != NULL && openUpvalues->location >= last) {
    ObjUpvalue* upvalue = openUpvalues;
    upvalue->closed = *upvalue->location;
    upvalue->location = &upvalue->closed;
    openUpvalues = upvalue->nextUpValue;
  }
}
