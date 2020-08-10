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

TEST(Object, allocateFunctionObject) {
  auto first = new Obj{};
  first->type = ObjType::OBJ_NATIVE;
  auto list = &first;
  auto function = allocateFunctionObject(list);
  ASSERT_EQ(*list, function);
  ASSERT_EQ((*list)->next->type, ObjType::OBJ_NATIVE);
}

TEST(Object, allocateClosureObject) {
  auto first = new Obj{};
  auto list = &first;

  auto function = allocateFunctionObject(list);
  function->upvalueCount = 100;
  auto closure = allocateClosureObject(function, list);
  ASSERT_EQ(*list, closure);
  ASSERT_EQ(closure->function, function);
  ASSERT_EQ(closure->upvalues.size(), 100);
  ASSERT_EQ(closure->upvalueCount, 100);
  ASSERT_EQ((*list)->next->type, ObjType::OBJ_FUNCTION);
}

Value tmp(int argCount, Value* args) { return NUMBER_VAL(100); }

TEST(Object, allocateNativeFnctionObject) {
  auto first = new Obj{};
  first->type = ObjType::OBJ_NATIVE;
  auto list = &first;

  NativeFunctionPtr ptr = &tmp;
  auto obj = allocateNativeFnctionObject(ptr, list);
  ASSERT_EQ(*list, obj);
  ASSERT_EQ((*list)->next->type, ObjType::OBJ_NATIVE);
  ASSERT_EQ(obj->func(0, nullptr).number, 100);
}

TEST(Object, allocateUpvalueObject) {
  auto first = new Obj{};
  auto list = &first;

  auto location = new Value{};
  auto upvalue = allocateUpvalueObject(location, list);
  ASSERT_EQ(*list, upvalue);
  ASSERT_EQ(upvalue->location, location);
  ASSERT_EQ(upvalue->nextUpValue, nullptr);
  ASSERT_EQ(upvalue->closed.type, ValueType::VAL_NIL);
}
