#include "main/compiler.hpp"

#include <gtest/gtest.h>

#include "main/value.hpp"

TEST(Compiler, Constructor) {
  const char* src = "abcde";
  auto chunk = new Chunk;
  auto compiler = Compiler(src, chunk);

  EXPECT_EQ(compiler.chunk, chunk);
  EXPECT_EQ(compiler.scanner.start, src);
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
  Value value = NUMBER_VAL(1.1);
  compiler.parser.previous.line = 255;
  compiler.emitConstant(value);
  EXPECT_EQ(compiler.chunk->code[0], OptCode::OP_CONSTANT);
  EXPECT_EQ(compiler.chunk->code[1], 0);
  EXPECT_DOUBLE_EQ(compiler.chunk->constants.peek()->number, value.number);
}

TEST(Compiler, makeConstant) {
  auto compiler = Compiler("this", new Chunk);
  Value value = NUMBER_VAL(1.1);
  EXPECT_EQ(compiler.makeConstant(value), 0);
  EXPECT_DOUBLE_EQ(compiler.chunk->constants.peek()->number, value.number);
  EXPECT_EQ(compiler.chunk->constants.values.size(), 1);
}

TEST(Compiler, number) {
  auto compiler = Compiler("1.1", new Chunk);
  compiler.advance(), compiler.advance();

  number(&compiler);
  EXPECT_EQ(compiler.chunk->constants.values.size(), 1);
  EXPECT_DOUBLE_EQ(compiler.chunk->constants.peek()->number, 1.1);
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
#define run(op, opt_code, is_pair, second_opt_code)                    \
  {                                                                    \
    auto compiler = new Compiler("1" #op "2", new Chunk);              \
    compiler->advance();                                               \
    binary(compiler);                                                  \
    EXPECT_EQ(compiler->chunk->constants.values.size(), 2);            \
    EXPECT_DOUBLE_EQ(compiler->chunk->constants.values[0].number, 1);  \
    EXPECT_DOUBLE_EQ(compiler->chunk->constants.values[1].number, 2);  \
    EXPECT_EQ(compiler->chunk->code.size(), is_pair ? 6 : 5);          \
    EXPECT_EQ(compiler->chunk->code[0], OptCode::OP_CONSTANT);         \
    EXPECT_EQ(compiler->chunk->code[1], 0);                            \
    EXPECT_EQ(compiler->chunk->code[2], OptCode::OP_CONSTANT);         \
    EXPECT_EQ(compiler->chunk->code[3], 1);                            \
    EXPECT_EQ(compiler->chunk->code[4], opt_code);                     \
    if (is_pair) EXPECT_EQ(compiler->chunk->code[5], second_opt_code); \
  }

  run(+, OptCode::OP_ADD, false, NULL);
  run(-, OptCode::OP_SUBTRACT, false, NULL);
  run(*, OptCode::OP_MULTIPLY, false, NULL);
  run(/, OptCode::OP_DIVIDE, false, NULL);
  run(==, OptCode::OP_EQUAL, false, NULL);
  run(>, OptCode::OP_GREATER, false, NULL);
  run(<, OptCode::OP_LESS, false, NULL);
  run(<=, OptCode::OP_GREATER, true, OptCode::OP_NOT);
  run(>=, OptCode::OP_LESS, true, OptCode::OP_NOT);
  run(!=, OptCode::OP_EQUAL, true, OptCode::OP_NOT);
#undef run
}

TEST(Compiler, unary) {
  {
    auto compiler = new Compiler("-100", new Chunk);
    compiler->advance();  // previous on -
    unary(compiler);
    EXPECT_EQ(compiler->chunk->constants.values.size(), 1);
    EXPECT_DOUBLE_EQ(compiler->chunk->constants.peek()->number, 100);
    EXPECT_EQ(compiler->chunk->code.size(), 3);
    EXPECT_EQ(compiler->chunk->code[0], OptCode::OP_CONSTANT);
    EXPECT_EQ(compiler->chunk->code[1], 0);
    EXPECT_EQ(compiler->chunk->code[2], OptCode::OP_NEGATE);
  }
  {
    auto compiler = new Compiler("!true", new Chunk);
    compiler->advance();  // previous on !
    unary(compiler);
    ASSERT_EQ(compiler->chunk->code.size(), 2);
    EXPECT_EQ(compiler->chunk->code[0], OptCode::OP_TRUE);
    EXPECT_EQ(compiler->chunk->code[1], OptCode::OP_NOT);
  }
}

TEST(Compiler, literal) {
  auto compiler = new Compiler("false true nil", new Chunk);
  compiler->advance(), compiler->advance();
  literal(compiler);
  ASSERT_TRUE(compiler->chunk->code.size() > 0);
  EXPECT_EQ(compiler->chunk->code[0], OptCode::OP_FALSE);
  compiler->advance();
  literal(compiler);
  EXPECT_EQ(compiler->chunk->code[1], OptCode::OP_TRUE);
  compiler->advance();
  literal(compiler);
  EXPECT_EQ(compiler->chunk->code[2], OptCode::OP_NIL);
}

TEST(Compiler, string) {
  std::string raw = "\"this is string\"";
  auto compiler = new Compiler(raw.c_str(), new Chunk);
  compiler->advance(), compiler->advance();
  string(compiler);
  ASSERT_EQ(compiler->chunk->code.size(), 2);
  ASSERT_EQ(compiler->chunk->constants.values.size(), 1);
  EXPECT_EQ(compiler->chunk->code[0], OptCode::OP_CONSTANT);
  EXPECT_EQ(compiler->chunk->code[1], 0);

  auto val = compiler->chunk->constants.values[0];
  ASSERT_TRUE(IS_OBJ(val));
  EXPECT_EQ(AS_STRING(val)->str, raw);
}

TEST(Compiler, expression) {
  {
    auto compiler = new Compiler("1+(2*3)", new Chunk);
    compiler->advance();  // current on 1
    compiler->expression();
    EXPECT_EQ(compiler->chunk->constants.values.size(), 3);
    EXPECT_DOUBLE_EQ(compiler->chunk->constants.values[0].number, 1);
    EXPECT_DOUBLE_EQ(compiler->chunk->constants.values[1].number, 2);
    EXPECT_DOUBLE_EQ(compiler->chunk->constants.values[2].number, 3);

    EXPECT_EQ(compiler->chunk->code.size(), 8);
    EXPECT_EQ(compiler->chunk->code[0], OptCode::OP_CONSTANT);
    EXPECT_EQ(compiler->chunk->code[1], 0);

    EXPECT_EQ(compiler->chunk->code[2], OptCode::OP_CONSTANT);
    EXPECT_EQ(compiler->chunk->code[3], 1);

    EXPECT_EQ(compiler->chunk->code[4], OptCode::OP_CONSTANT);
    EXPECT_EQ(compiler->chunk->code[5], 2);

    EXPECT_EQ(compiler->chunk->code[6], OptCode::OP_MULTIPLY);
    EXPECT_EQ(compiler->chunk->code[7], OptCode::OP_ADD);
  }
  {
    auto compiler = new Compiler("1+(2*3-1.1)", new Chunk);
    compiler->advance();  // current on 1
    compiler->expression();
    EXPECT_EQ(compiler->chunk->constants.values.size(), 4);
    EXPECT_DOUBLE_EQ(compiler->chunk->constants.values[0].number, 1);
    EXPECT_DOUBLE_EQ(compiler->chunk->constants.values[1].number, 2);
    EXPECT_DOUBLE_EQ(compiler->chunk->constants.values[2].number, 3);
    EXPECT_DOUBLE_EQ(compiler->chunk->constants.values[3].number, 1.1);

    EXPECT_EQ(compiler->chunk->code.size(), 11);
    EXPECT_EQ(compiler->chunk->code[0], OptCode::OP_CONSTANT);
    EXPECT_EQ(compiler->chunk->code[1], 0);

    EXPECT_EQ(compiler->chunk->code[2], OptCode::OP_CONSTANT);
    EXPECT_EQ(compiler->chunk->code[3], 1);

    EXPECT_EQ(compiler->chunk->code[4], OptCode::OP_CONSTANT);
    EXPECT_EQ(compiler->chunk->code[5], 2);

    EXPECT_EQ(compiler->chunk->code[6], OptCode::OP_MULTIPLY);

    EXPECT_EQ(compiler->chunk->code[7], OptCode::OP_CONSTANT);
    EXPECT_EQ(compiler->chunk->code[8], 3);
    EXPECT_EQ(compiler->chunk->code[9], OptCode::OP_SUBTRACT);
    EXPECT_EQ(compiler->chunk->code[10], OptCode::OP_ADD);
  }
}

TEST(Compiler, parsePrecedence) {
  auto compiler = new Compiler("-1.1+1000", new Chunk);
  compiler->advance();  // current on -
  compiler->parsePrecedence(Precedence::PREC_TERM);
  EXPECT_EQ(compiler->chunk->constants.values.size(), 1);
  EXPECT_DOUBLE_EQ(compiler->chunk->constants.peek()->number, 1.1);

  EXPECT_EQ(compiler->chunk->code.size(), 3);
  EXPECT_EQ(compiler->chunk->code[0], OptCode::OP_CONSTANT);
  EXPECT_EQ(compiler->chunk->code[1], 0);
  EXPECT_EQ(compiler->chunk->code[2], OptCode::OP_NEGATE);
}
