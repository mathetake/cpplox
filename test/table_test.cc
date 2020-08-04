#include "main/table.hpp"

#include <gtest/gtest.h>

TEST(Table, get) {
  auto table = Table{};
  auto exists = new ObjString("aaa");
  table.set(exists, NUMBER_VAL(1111));
  EXPECT_EQ(table.count, 1);

  auto actual = new Value{};
  ASSERT_TRUE(table.get(exists, actual));
  ASSERT_EQ(actual->number, 1111);
  ASSERT_FALSE(table.get(new ObjString("not exists"), new Value{}));
}

TEST(Table, set) {
  auto table = Table{};
  table.set(new ObjString("aaa"), BOOL_VAL(false));
  EXPECT_EQ(table.count, 1);
}

TEST(Table, deleteEntry) {
  auto table = Table{};
  auto exists = new ObjString("bbb");
  auto deleteTarget = new ObjString("aaa");
  table.set(exists, NUMBER_VAL(1111));
  table.set(deleteTarget, NUMBER_VAL(1111));

  ASSERT_TRUE(findEntry(table.entries, exists)->key);
  ASSERT_TRUE(findEntry(table.entries, deleteTarget)->key);

  table.deleteKey(deleteTarget);
  ASSERT_TRUE(findEntry(table.entries, exists)->key);
  ASSERT_FALSE(findEntry(table.entries, deleteTarget)->key);
}

TEST(Table, findString) {
  auto table = Table{};
  auto target = new ObjString("key");
  table.set(target, NUMBER_VAL(1111));
  ASSERT_EQ(table.findString(new ObjString("key")), target);

  ASSERT_FALSE(table.findString(new ObjString("not-exists")));
}

TEST(Table, findEntry) {
  auto table = Table{};
  auto key = new ObjString("key");
  Entry* found = findEntry(table.entries, key);
  EXPECT_FALSE(found->key);
  table.set(key, BOOL_VAL(false));
  EXPECT_EQ(table.count, 1);

  found = findEntry(table.entries, key);
  ASSERT_EQ(found->key->str, "key");
}

TEST(Table, adjustCapacity) {
  auto table = Table{};
  auto key = new ObjString("key");
  table.set(key, BOOL_VAL(true));
  EXPECT_EQ(table.count, 1);
  auto found = findEntry(table.entries, key);
  ASSERT_EQ(found->key->str, "key");

  int expCap = 1000;
  table.adjustCapacity(expCap);

  EXPECT_EQ(table.count, 1);
  EXPECT_EQ(table.entries->capacity(), expCap);
  found = findEntry(table.entries, key);
  ASSERT_TRUE(found->key);
  ASSERT_EQ(found->key->str, "key");
}

TEST(Table, addAll) {
  auto dst = Table{};
  auto a = new ObjString("a");
  dst.set(a, BOOL_VAL(true));
  ASSERT_EQ(dst.count, 1);

  auto src = Table{};
  auto b = new ObjString("b");
  auto c = new ObjString("c");
  src.set(b, BOOL_VAL(true)), src.set(c, BOOL_VAL(true));
  ASSERT_EQ(src.count, 2);

  dst.addAll(&src);
  ASSERT_EQ(dst.count, 3);

  ASSERT_TRUE(findEntry(dst.entries, a)->key);
  ASSERT_TRUE(findEntry(dst.entries, b)->key);
  ASSERT_TRUE(findEntry(dst.entries, c)->key);
  ASSERT_FALSE(findEntry(dst.entries, new ObjString("not found"))->key);
}
