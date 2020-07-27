
#include "chunk.hpp"

#include "value.hpp"

void Chunk::write_chunk(uint8_t byte, int line) {
  code.push_back(byte);
  lines.push_back(line);
}

int Chunk::add_const(Value value) {
  constants.writeValueArray(value);
  return constants.values.size() - 1;
}

uint8_t* Chunk::peek_code() {
  if (code.empty()) {
    return nullptr;
  } else {
    return &code[code.size() - 1];
  }
}
