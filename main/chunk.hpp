#ifndef cpplox_chunk_h
#define cpplox_chunk_h

#include "common.hpp"
#include "value.hpp"

enum OptCode : uint8_t {
  OP_RETURN,
  OP_NEGATE,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_CONSTANT,
};

class Chunk {
 public:
  ValueArray constants;
  std::vector<uint8_t> code;
  std::vector<int> lines;
  Chunk() {
    code = std::vector<uint8_t>();
    constants = ValueArray{};
  };
  ~Chunk(){};

  void write_chunk(uint8_t byte, int line);
  int add_const(Value value);
  size_t count() { return code.size(); };
  uint8_t* peek_code();
};

#endif
