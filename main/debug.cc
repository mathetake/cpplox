#include "debug.hpp"

#include "object.hpp"
#include "value.hpp"

void disassembleChunk(Chunk* chunk, const char* name) {
  printf("== %s ==\n", name);

  for (size_t i = 0; i < chunk->count();) {
    i = disassembleInstruction(chunk, i);
  };
}

int simpleInstruction(const char* name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}

int jumpInstruction(const char* name, int sign, Chunk* chunk, int offset) {
  uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
  jump |= chunk->code[offset + 2];
  printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
  return offset + 3;
}

int constantInstruction(const char* name, Chunk* chunk, int offset) {
  auto index = chunk->code[offset + 1];
  printf("%-16s %4d '", name, index);
  printValue(chunk->constants.values[index]);
  printf("\n");
  return offset + 2;
}

int byteInstruction(const char* name, Chunk* chunk, int offset) {
  uint8_t slot = chunk->code[offset + 1];
  printf("%-16s %4d\n", name, slot);
  return offset + 2;
}

int disassembleInstruction(Chunk* chunk, int offset) {
  printf("%04d ", offset);
  if (offset > 0 && chunk->lines[offset - 1] == chunk->lines[offset]) {
    printf("   | ");
  } else {
    printf("%4d ", chunk->lines[offset]);
  }
  auto inst = chunk->code[offset];
  switch (inst) {
    case OptCode::OP_RETURN:
      return simpleInstruction("OP_RETURN", offset);
    case OptCode::OP_CONSTANT:
      return constantInstruction("OP_CONST", chunk, offset);
    case OptCode::OP_ADD:
      return simpleInstruction("OP_ADD", offset);
    case OptCode::OP_SUBTRACT:
      return simpleInstruction("OP_SUBTRACT", offset);
    case OptCode::OP_MULTIPLY:
      return simpleInstruction("OP_MULTIPLY", offset);
    case OptCode::OP_DIVIDE:
      return simpleInstruction("OP_DIVIDE", offset);
    case OptCode::OP_NEGATE:
      return simpleInstruction("OP_NEGATE", offset);
    case OptCode::OP_NOT:
      return simpleInstruction("OP_NOT", offset);
    case OptCode::OP_NIL:
      return simpleInstruction("OP_NIL", offset);
    case OptCode::OP_TRUE:
      return simpleInstruction("OP_TRUE", offset);
    case OptCode::OP_FALSE:
      return simpleInstruction("OP_FALSE", offset);
    case OptCode::OP_EQUAL:
      return simpleInstruction("OP_EQUAL", offset);
    case OptCode::OP_GREATER:
      return simpleInstruction("OP_GREATER", offset);
    case OptCode::OP_LESS:
      return simpleInstruction("OP_LESS", offset);
    case OptCode::OP_PRINT:
      return simpleInstruction("OP_PRINT", offset);
    case OptCode::OP_POP:
      return simpleInstruction("OP_POP", offset);
    case OptCode::OP_DEFINE_GLOBAL:
      return constantInstruction("OP_DEFINE_GLOBAL", chunk, offset);
    case OptCode::OP_GET_GLOBAL:
      return constantInstruction("OP_GET_GLOBAL", chunk, offset);
    case OptCode::OP_SET_GLOBAL:
      return constantInstruction("OP_SET_GLOBAL", chunk, offset);
    case OptCode::OP_GET_LOCAL:
      return byteInstruction("OP_GET_LOCAL", chunk, offset);
    case OptCode::OP_SET_LOCAL:
      return byteInstruction("OP_SET_LOCAL", chunk, offset);
    case OptCode::OP_JUMP:
      return jumpInstruction("OP_JUMP", 1, chunk, offset);
    case OptCode::OP_JUMP_IF_FALSE:
      return jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
    case OptCode::OP_LOOP:
      return jumpInstruction("OP_LOOP", -1, chunk, offset);
    case OptCode::OP_CALL:
      return byteInstruction("OP_CALL", chunk, offset);
    case OptCode::OP_GET_UPVALUE:
      return byteInstruction("OP_GET_UPVALUE", chunk, offset);
    case OptCode::OP_SET_UPVALUE:
      return byteInstruction("OP_SET_UPVALUE", chunk, offset);
    case OptCode::OP_CLOSE_UPVALUE:
      return simpleInstruction("OP_CLOSE_UPVALUE", offset);
    case OptCode::OP_CLOSURE: {
      offset++;
      uint8_t constant = chunk->code[offset++];
      printf("%-16s %4d ", "OP_CLOSURE", constant);
      printValue(chunk->constants.values[constant]);
      printf("\n");

      ObjFunction* function = AS_FUNCTION(chunk->constants.values[constant]);
      for (int j = 0; j < function->upvalueCount; j++) {
        int isLocal = chunk->code[offset++];
        int index = chunk->code[offset++];
        printf("%04d      |                     %s %d\n", offset - 2,
               isLocal ? "local" : "upvalue", index);
      }

      return offset;
    }
    default:
      printf("unknown optcode: %d\n", inst);
      return offset + 1;
  }
};
