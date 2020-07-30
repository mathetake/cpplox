#include "main/chunk.hpp"

#include <gtest/gtest.h>

#include "main/value.hpp"

TEST(Chunk, constructor) {
  auto c = new Chunk;
  EXPECT_EQ(c->code.size(), 0) << "must be zero length vector";
  delete c;
}

TEST(Chunk, write_chunk) {
  auto c = new Chunk;
  uint8_t b = 100;
  int line = 10000;
  c->write_chunk(b, line);
  EXPECT_EQ(c->code.size(), 1);
  EXPECT_EQ(c->lines.size(), 1);
  EXPECT_EQ(c->lines[0], line);

  auto actual = c->peek_code();
  EXPECT_NE(actual, nullptr) << "v must no be nullptr";
  EXPECT_EQ(*actual, b) << "value must be `" << b;
}

TEST(Chunk, add_const) {
  auto c = new Chunk;
  Value* actual = c->constants.peek();
  EXPECT_EQ(actual, nullptr);

  Value v = NUMBER_VAL(100);
  auto index = c->add_const(v);
  EXPECT_EQ(index, 0);
  actual = c->constants.peek();
  EXPECT_NE(actual, nullptr);
  EXPECT_EQ(actual->number, v.number);
}
