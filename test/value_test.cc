#include "main/value.hpp"

#include <gtest/gtest.h>

#include "main/object.hpp"

TEST(Value, writeValueArray) {
  auto array = new ValueArray();
  auto v = NUMBER_VAL(100);
  array->writeValueArray(v);
  EXPECT_EQ(v.number, array->peek()->number);
}

TEST(Value, valuesEqual) {
  EXPECT_FALSE(valuesEqual(NUMBER_VAL(1231), NIL_VAL));
  EXPECT_FALSE(valuesEqual(BOOL_VAL(false), NIL_VAL));
  EXPECT_FALSE(valuesEqual(BOOL_VAL(false), BOOL_VAL(true)));
  EXPECT_TRUE(valuesEqual(BOOL_VAL(true), BOOL_VAL(true)));
  EXPECT_TRUE(valuesEqual(BOOL_VAL(false), BOOL_VAL(false)));
  EXPECT_TRUE(valuesEqual(NUMBER_VAL(1231), NUMBER_VAL(1231)));
  EXPECT_FALSE(valuesEqual(NUMBER_VAL(1231), NUMBER_VAL(-1)));

  EXPECT_TRUE(valuesEqual(OBJ_VAL(allocateStringObject("abcd", 4)),
                          OBJ_VAL(allocateStringObject("abcd", 4))));
  EXPECT_FALSE(valuesEqual(OBJ_VAL(allocateStringObject("abcd", 4)),
                           OBJ_VAL(allocateStringObject("aaa", 3))));
}
