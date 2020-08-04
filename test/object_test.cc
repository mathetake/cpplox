#include "main/object.hpp"

#include <gtest/gtest.h>

#include "main/vm.hpp"

TEST(Object, printObject) { printObject(OBJ_VAL(new ObjString("aaaa"))); }

TEST(Object, isObjType) {
  EXPECT_TRUE(
      isObjType(Value{.type = ValueType::VAL_OBJ, .obj = new ObjString("aaaa")},
                OBJ_STRING));
  EXPECT_FALSE(isObjType(Value{.type = ValueType::VAL_NIL}, OBJ_STRING));
}

TEST(Object, allocateStringObject) {
  auto strings = new Table{};
  Obj* objs = new Obj{};
  auto first = allocateStringObject("abcd", 4, strings, &objs);
  EXPECT_EQ(first->type, OBJ_STRING);
  EXPECT_EQ(first->str.size(), 4);
  EXPECT_EQ(first->str, "abcd");

  EXPECT_EQ(objs, (Obj*)first);
  EXPECT_EQ(strings->count, 1);
  EXPECT_EQ(strings->findString(new ObjString("abcd")), first);

  auto second = allocateStringObject("efgh", 4, strings, &objs);
  EXPECT_EQ(strings->findString(new ObjString("efgh")), second);
  ASSERT_EQ(objs, (Obj*)second);
  ASSERT_EQ(second->next, first);
}

TEST(Object, hashString) {
  auto a = hashString("a", 1);
  auto b = hashString("b", 1);

  EXPECT_EQ(a, hashString("a", 1));
  EXPECT_NE(a, b);
}