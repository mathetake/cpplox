#include "main/vm.hpp"

#include <gtest/gtest.h>

TEST(VM, interpret) {
  auto c = new Chunk;
  int begin = 10, end = 13;
  for (int v = begin; v < end; ++v) c->write_chunk(v, 0);

  VM vm_local{};
  vm_local.interpret(c);
  EXPECT_EQ(vm_local.chunk, c);
  for (int v = begin; v < end; ++v, ++vm_local.ip) EXPECT_EQ(*vm_local.ip, v);
}

TEST(VM, run_arithmetic) {
  auto c = new Chunk;
  int constant = c->add_const(1.2);
  c->write_chunk(OptCode::OP_CONSTANT, 123);
  c->write_chunk(constant, 123);

  constant = c->add_const(3.4);
  c->write_chunk(OptCode::OP_CONSTANT, 123);
  c->write_chunk(constant, 123);

  c->write_chunk(OptCode::OP_ADD, 123);

  constant = c->add_const(8);
  c->write_chunk(OptCode::OP_CONSTANT, 123);
  c->write_chunk(constant, 123);

  c->write_chunk(OptCode::OP_DIVIDE, 123);

  c->write_chunk(OptCode::OP_NEGATE, 123);
  c->write_chunk(OptCode::OP_RETURN, 123);

  VM vm_local{};
  vm_local.interpret(c);
  EXPECT_EQ(vm_local.chunk, c);
  EXPECT_EQ(vm_local.run(), IntepretResult::INTERPRET_OK);
  EXPECT_DOUBLE_EQ(*(vm_local.stack_top - 1), -0.575);  // -(1.2+3.4)/5.6
}

TEST(VM, initVM) {
  VM vm_local{};
  vm_local.initVM();
  EXPECT_NE(vm_local.stack_top, nullptr);
  EXPECT_NE(vm_local.stack, nullptr);
  vm_local.stack[100] = 1.1;
  EXPECT_EQ(vm_local.stack[100], 1.1);
}

TEST(VM, push) {
  VM vm_local{};
  vm_local.initVM();
  double exp = 1.1;
  vm_local.push(exp);

  EXPECT_EQ(*(vm_local.stack_top - 1), exp);
  EXPECT_EQ(vm_local.stack_top - vm_local.stack, 1);
}

TEST(VM, pop) {
  VM vm_local{};
  vm_local.initVM();
  double exp = 1.1;
  vm_local.push(exp);
  vm_local.push(exp);

  auto actual = vm_local.pop();

  EXPECT_EQ(actual, exp);
  EXPECT_EQ(vm_local.stack_top - vm_local.stack, 1);

  actual = vm_local.pop();
  EXPECT_EQ(actual, exp);
  EXPECT_EQ(vm_local.stack, vm_local.stack_top);
}

TEST(VM, binary_op) {
  VM vm_local{};
  Chunk* c;

#define run(OP_CODE)            \
  vm_local.initVM();            \
  vm_local.push(10);            \
  vm_local.push(100);           \
  c = new Chunk;                \
  c->write_chunk(OP_CODE, 123); \
  vm_local.interpret(c);        \
  vm_local.run();

  // add
  run(OptCode::OP_ADD);
  EXPECT_DOUBLE_EQ(110, *(vm_local.stack_top - 1)) << "must be 110";

  // sub
  run(OptCode::OP_SUBTRACT);
  EXPECT_DOUBLE_EQ(-90, *(vm_local.stack_top - 1)) << "must be -90";

  // mul
  run(OptCode::OP_MULTIPLY);
  EXPECT_DOUBLE_EQ(1000, *(vm_local.stack_top - 1)) << "must be 1000";

  // sub
  run(OptCode::OP_DIVIDE);
  EXPECT_DOUBLE_EQ(0.1, *(vm_local.stack_top - 1)) << "must be 0.1";

#undef run
}
