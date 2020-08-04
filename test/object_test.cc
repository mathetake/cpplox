#include "main/object.hpp"

#include <gtest/gtest.h>

#include "main/vm.hpp"

TEST(Object, printObject) {
  const char* raw = "abcd";
  auto actual = allocateStringObject(raw, 4);
  printObject(OBJ_VAL(actual));
}

TEST(Object, isObjType) {
  EXPECT_TRUE(
      isObjType(Value{.type = ValueType::VAL_OBJ, .obj = new ObjString("aaaa")},
                OBJ_STRING));
  EXPECT_FALSE(isObjType(Value{.type = ValueType::VAL_NIL}, OBJ_STRING));
}

TEST(Object, allocateStringObject) {
  const char* raw = "abcd";
  auto actual = allocateStringObject(raw, 4);
  EXPECT_EQ(actual->type, OBJ_STRING);
  EXPECT_EQ(actual->str.size(), 4);
  EXPECT_EQ(actual->str, "abcd");

  EXPECT_EQ(vm.objects, (Obj*)actual);
  EXPECT_EQ(vm.strings.count, 2);
  EXPECT_EQ(vm.strings.findString(new ObjString("abcd")), actual);
}

TEST(Object, hashString) {
  auto a = hashString("a", 1);
  auto b = hashString("b", 1);

  EXPECT_EQ(a, hashString("a", 1));
  EXPECT_NE(a, b);
}