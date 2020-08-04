#include "main/vm.hpp"

#include <gtest/gtest.h>

#include "main/object.hpp"
#include "main/value.hpp"

TEST(VM, interpret) {
  auto c = new Chunk;
  int begin = 10, end = 13;
  for (int v = begin; v < end; ++v) c->write_chunk(v, 0);

  VM vm_local{};
  vm_local.interpret(c);
  EXPECT_EQ(vm_local.chunk, c);
  for (int v = begin; v < end; ++v, ++vm_local.ip) EXPECT_EQ(*vm_local.ip, v);
}

TEST(VM, OP_DEFINE_GLOBAL) {
  auto variable = new ObjString("variable name");
  auto c = new Chunk;
  c->write_chunk(OptCode::OP_DEFINE_GLOBAL, 123);
  c->write_chunk(c->add_const(OBJ_VAL(variable)), 123);
  c->write_chunk(OptCode::OP_RETURN, 123);
  VM vm_local{};
  vm_local.interpret(c);
  vm_local.push(NUMBER_VAL(1.2));
  vm_local.run();

  ASSERT_EQ(vm_local.globals.count, 1);
  Value actual;
  ASSERT_TRUE(vm_local.globals.get(variable, &actual));
  EXPECT_DOUBLE_EQ(actual.number, 1.2);
}

TEST(VM, OP_GET_GLOBAL) {
  auto variable = new ObjString("variable name");
  auto c = new Chunk;
  c->write_chunk(OptCode::OP_GET_GLOBAL, 123);
  c->write_chunk(c->add_const(OBJ_VAL(variable)), 123);
  c->write_chunk(OptCode::OP_RETURN, 123);

  VM vm_local{};
  vm_local.globals.set(variable, NUMBER_VAL(1.2));
  vm_local.interpret(c);
  vm_local.run();
  EXPECT_DOUBLE_EQ(vm_local.peek(0).number, 1.2);
}

TEST(VM, run_arithmetic) {
  auto c = new Chunk;
  int constant = c->add_const(NUMBER_VAL(1.2));
  c->write_chunk(OptCode::OP_CONSTANT, 123);
  c->write_chunk(constant, 123);

  constant = c->add_const(NUMBER_VAL(3.4));
  c->write_chunk(OptCode::OP_CONSTANT, 123);
  c->write_chunk(constant, 123);

  c->write_chunk(OptCode::OP_ADD, 123);

  constant = c->add_const(NUMBER_VAL(8));
  c->write_chunk(OptCode::OP_CONSTANT, 123);
  c->write_chunk(constant, 123);

  c->write_chunk(OptCode::OP_DIVIDE, 123);

  c->write_chunk(OptCode::OP_NEGATE, 123);
  c->write_chunk(OptCode::OP_RETURN, 123);

  VM vm_local{};
  vm_local.interpret(c);
  EXPECT_EQ(vm_local.chunk, c);
  EXPECT_EQ(vm_local.run(), IntepretResult::INTERPRET_OK);
  EXPECT_DOUBLE_EQ((vm_local.stack_top - 1)->number, -0.575);  // -(1.2+3.4)/5.6
}

TEST(VM, initVM) {
  VM vm_local{};
  vm_local.initVM();
  EXPECT_NE(vm_local.stack_top, nullptr);
  EXPECT_NE(vm_local.stack, nullptr);
  vm_local.stack[100].number = 1.1;
  EXPECT_DOUBLE_EQ(vm_local.stack[100].number, 1.1);
}

TEST(VM, push) {
  VM vm_local{};
  vm_local.initVM();
  Value exp = NUMBER_VAL(1.1);
  vm_local.push(exp);

  EXPECT_EQ((vm_local.stack_top - 1)->number, exp.number);
  EXPECT_EQ(vm_local.stack_top - vm_local.stack, 1);
}

TEST(VM, pop) {
  VM vm_local{};
  vm_local.initVM();
  Value exp = NUMBER_VAL(1.1);
  vm_local.push(exp);
  vm_local.push(exp);

  auto actual = vm_local.pop();

  EXPECT_DOUBLE_EQ(actual.number, exp.number);
  EXPECT_EQ(vm_local.stack_top - vm_local.stack, 1);

  actual = vm_local.pop();
  EXPECT_DOUBLE_EQ(actual.number, exp.number);
  EXPECT_EQ(vm_local.stack, vm_local.stack_top);
}

TEST(VM, binary_op) {
  VM vm_local{};
  Chunk* c;

#define run(OP_CODE, v1, v2)     \
  vm_local.initVM();             \
  vm_local.push(NUMBER_VAL(v1)); \
  vm_local.push(NUMBER_VAL(v2)); \
  c = new Chunk;                 \
  c->write_chunk(OP_CODE, 123);  \
  vm_local.interpret(c);         \
  vm_local.run();

  // add
  run(OptCode::OP_ADD, 10, 100);
  EXPECT_DOUBLE_EQ(110, (vm_local.stack_top - 1)->number) << "must be 110";

  // sub
  run(OptCode::OP_SUBTRACT, 10, 100);
  EXPECT_DOUBLE_EQ(-90, (vm_local.stack_top - 1)->number) << "must be -90";

  // mul
  run(OptCode::OP_MULTIPLY, 10, 100);
  EXPECT_DOUBLE_EQ(1000, (vm_local.stack_top - 1)->number) << "must be 1000";

  // sub
  run(OptCode::OP_DIVIDE, 10, 100);
  EXPECT_DOUBLE_EQ(0.1, (vm_local.stack_top - 1)->number) << "must be 0.1";

  // ==
  run(OptCode::OP_EQUAL, 10, 100);
  EXPECT_FALSE((vm_local.stack_top - 1)->boolean);

  // ==
  run(OptCode::OP_EQUAL, 100, 100);
  EXPECT_TRUE((vm_local.stack_top - 1)->boolean);

  // >
  run(OptCode::OP_GREATER, 100, 10);
  EXPECT_TRUE((vm_local.stack_top - 1)->boolean);

  // >
  run(OptCode::OP_GREATER, 10, 100);
  EXPECT_FALSE((vm_local.stack_top - 1)->boolean);

  // <
  run(OptCode::OP_LESS, 10, 100);
  EXPECT_TRUE((vm_local.stack_top - 1)->boolean);

  // <
  run(OptCode::OP_LESS, 100, 10);
  EXPECT_FALSE((vm_local.stack_top - 1)->boolean);
#undef run
}

TEST(VM, isFalsy) {
  EXPECT_FALSE(VM::isFalsy(NUMBER_VAL(1.1)));
  EXPECT_TRUE(VM::isFalsy(NIL_VAL));
  EXPECT_TRUE(VM::isFalsy(BOOL_VAL(false)));
  EXPECT_FALSE(VM::isFalsy(BOOL_VAL(true)));
}

TEST(VM, concatenate) {
  VM vm_local{};
  vm_local.initVM();
  vm_local.push(OBJ_VAL(new ObjString("ab")));
  vm_local.push(OBJ_VAL(new ObjString("cd")));
  auto a = std::string("aaa", 1);
  EXPECT_EQ(a.size(), 1);

  vm_local.concatenate();
  EXPECT_EQ(AS_STRING(vm_local.pop())->str, "abcd");
}

TEST(VM, freeVM) {
  VM vm_local{};
  vm_local.initVM();
  Obj* first = new Obj{};
  Obj* second = new Obj{};

  first->next = second;
  vm_local.objects = first;
  vm_local.freeVM();
  EXPECT_EQ(vm_local.objects, nullptr);
}
