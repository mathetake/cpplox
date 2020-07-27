#include "main/value.hpp"

#include <gtest/gtest.h>

TEST(ValueArray, writeValueArray) {
  auto array = new ValueArray();
  auto v = 100;
  array->writeValueArray(v);
  EXPECT_EQ(v, *array->peek());
}
