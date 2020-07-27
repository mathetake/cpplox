#include "debug.hpp"

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

int constantInstruction(const char* name, Chunk* chunk, int offset) {
  auto index = chunk->code[offset + 1];
  printf("%-16s %4d '", name, index);
  printValue(chunk->constants.values[index]);
  printf("\n");
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
    default:
      printf("unknown optcode: %d\n", inst);
      return offset + 1;
  }
};
