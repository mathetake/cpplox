#ifndef cpplox_chunk_h
#define cpplox_chunk_h

#include "common.hpp"
#include "value.hpp"

enum OptCode : uint8_t {
  OP_RETURN,
  OP_NOT,
  OP_NEGATE,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_CONSTANT,
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,
  OP_PRINT,
  OP_POP,
  OP_DEFINE_GLOBAL,
  OP_GET_GLOBAL,
  OP_GET_LOCAL,
  OP_SET_GLOBAL,
  OP_SET_LOCAL,
  OP_JUMP_IF_FALSE,
  OP_JUMP,
  OP_LOOP,
  OP_CALL,
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
