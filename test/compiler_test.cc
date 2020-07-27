#include "main/compiler.hpp"

#include <gtest/gtest.h>

TEST(Compiler, Constructor) {
  const char* src = "abcde";
  auto chunk = new Chunk;
  auto compiler = Compiler(src, chunk);

  EXPECT_EQ(compiler.chunk, chunk);
  EXPECT_EQ(compiler.scanner.start, src);

  // check parse rule table
  auto actual = parseRules[TokenType::TOKEN_LEFT_PAREN];
  EXPECT_EQ(grouping, actual.prefix);
  EXPECT_FALSE(actual.infix);
  EXPECT_EQ(Precedence::PREC_NONE, actual.precedence);
}

TEST(Compiler, emitReturn) {
  auto compiler = Compiler("", new Chunk);
  compiler.emitReturn();
  EXPECT_EQ(1, compiler.chunk->code.size());
}

TEST(Compiler, endCompiler) {
  auto compiler = Compiler("", new Chunk);
  compiler.endCompiler();
  EXPECT_EQ(1, compiler.chunk->code.size());
}

TEST(Compiler, advance) {
  auto compiler = Compiler("this", new Chunk);
  compiler.advance();
  EXPECT_FALSE(compiler.parser.panicMode);

  compiler = Compiler("\"afa", new Chunk);
  compiler.advance();
  EXPECT_TRUE(compiler.parser.panicMode);
}

TEST(Compiler, consume) {
  const char* msg = "error";
  auto compiler = Compiler("this", new Chunk);
  compiler.parser.current = compiler.scanner.scanToken();
  compiler.consume(TokenType::TOKEN_THIS, msg);
  EXPECT_FALSE(compiler.parser.panicMode);

  compiler = Compiler("thi ", new Chunk);
  compiler.parser.current = compiler.scanner.scanToken();
  compiler.consume(TokenType::TOKEN_THIS, msg);
  EXPECT_TRUE(compiler.parser.panicMode);
}

TEST(Compiler, errorAt) {
  auto compiler = Compiler("this", new Chunk);
  compiler.errorAt(new Token{}, "msg");
  EXPECT_TRUE(compiler.parser.panicMode);
  EXPECT_TRUE(compiler.parser.hadError);
}

TEST(Compiler, emitByte) {
  auto compiler = Compiler("this", new Chunk);
  compiler.parser.previous.line = 255;
  compiler.emitByte(255);

  EXPECT_EQ(compiler.chunk->code[0], 255);
  EXPECT_EQ(compiler.chunk->lines[0], 255);
}

TEST(Compiler, emitConstant) {
  auto compiler = Compiler("this", new Chunk);
  Value value = 1.1;
  compiler.parser.previous.line = 255;
  compiler.emitConstant(value);
  EXPECT_EQ(compiler.chunk->code[0], OptCode::OP_CONSTANT);
  EXPECT_EQ(compiler.chunk->code[1], 0);
  EXPECT_EQ(*compiler.chunk->constants.peek(), value);
}

TEST(Compiler, makeConstant) {
  auto compiler = Compiler("this", new Chunk);
  Value value = 1.1;
  EXPECT_EQ(compiler.makeConstant(value), 0);
  EXPECT_EQ(*compiler.chunk->constants.peek(), value);
  EXPECT_EQ(compiler.chunk->constants.values.size(), 1);
}

TEST(Compiler, number) {
  auto compiler = Compiler("1.1", new Chunk);
  compiler.advance(), compiler.advance();

  number(&compiler);
  EXPECT_EQ(compiler.chunk->constants.values.size(), 1);
  EXPECT_EQ(*compiler.chunk->constants.peek(), 1.1);
}

TEST(Compiler, grouping) {
  auto compiler = new Compiler("(1+2)", new Chunk);
  compiler->advance();  // current on (
  grouping(compiler);
  EXPECT_EQ(compiler->chunk->constants.values.size(), 2);
  EXPECT_EQ(compiler->chunk->code.size(), 5);
  EXPECT_EQ(compiler->chunk->code[0], OptCode::OP_CONSTANT);
  EXPECT_EQ(compiler->chunk->code[1], 0);
  EXPECT_EQ(compiler->chunk->code[2], OptCode::OP_CONSTANT);
  EXPECT_EQ(compiler->chunk->code[3], 1);
  EXPECT_EQ(compiler->chunk->code[4], OptCode::OP_ADD);
}

TEST(Compiler, binary) {
#define run(op, opt_code)                                      \
  {                                                            \
    auto compiler = new Compiler("1" #op "2", new Chunk);      \
    compiler->advance();                                       \
    binary(compiler);                                          \
    EXPECT_EQ(compiler->chunk->constants.values.size(), 2);    \
    EXPECT_EQ(compiler->chunk->constants.values[0], 1);        \
    EXPECT_EQ(compiler->chunk->constants.values[1], 2);        \
    EXPECT_EQ(compiler->chunk->code.size(), 5);                \
    EXPECT_EQ(compiler->chunk->code[0], OptCode::OP_CONSTANT); \
    EXPECT_EQ(compiler->chunk->code[1], 0);                    \
    EXPECT_EQ(compiler->chunk->code[2], OptCode::OP_CONSTANT); \
    EXPECT_EQ(compiler->chunk->code[3], 1);                    \
    EXPECT_EQ(compiler->chunk->code[4], opt_code);             \
  }

  run(+, OptCode::OP_ADD);
  run(-, OptCode::OP_SUBTRACT);
  run(*, OptCode::OP_MULTIPLY);
  run(/, OptCode::OP_DIVIDE);

#undef run
}
TEST(Compiler, unary) {
  auto compiler = new Compiler("-100", new Chunk);
  compiler->advance();  // current on -
  unary(compiler);
  EXPECT_EQ(compiler->chunk->constants.values.size(), 1);
  EXPECT_EQ(*compiler->chunk->constants.peek(), 100);
  EXPECT_EQ(compiler->chunk->code.size(), 3);
  EXPECT_EQ(compiler->chunk->code[0], OptCode::OP_CONSTANT);
  EXPECT_EQ(compiler->chunk->code[1], 0);
  EXPECT_EQ(compiler->chunk->code[2], OptCode::OP_NEGATE);
}
