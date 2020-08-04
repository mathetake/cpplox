#include "main/compiler.hpp"

#include <gtest/gtest.h>

#include "main/value.hpp"

TEST(Compiler, check) {
  auto obj = new Obj{};
  auto compiler = Compiler("true", new Chunk, new Table{}, &obj);
  compiler.advance();
  ASSERT_TRUE(compiler.check(TokenType::TOKEN_TRUE));
  ASSERT_FALSE(compiler.check(TokenType::TOKEN_FALSE));
}

TEST(Compiler, match) {
  auto obj = new Obj{};
  auto compiler = Compiler("1011", new Chunk, new Table{}, &obj);
  compiler.advance();
  ASSERT_TRUE(compiler.match(TokenType::TOKEN_NUMBER));
  ASSERT_TRUE(compiler.match(TokenType::TOKEN_EOF));
}

TEST(Compiler, Constructor) {
  const char* src = "abcde";
  auto obj = new Obj{};
  auto chunk = new Chunk{};
  auto strTable = new Table{};
  auto compiler = Compiler(src, chunk, strTable, &obj);

  EXPECT_EQ(compiler.stringTable, strTable);
  EXPECT_EQ(compiler.objects, &obj);
  EXPECT_EQ(compiler.chunk, chunk);
  EXPECT_EQ(compiler.scanner.start, src);
}

TEST(Compiler, emitReturn) {
  auto obj = new Obj{};
  auto compiler = Compiler("", new Chunk, new Table{}, &obj);
  compiler.emitReturn();
  EXPECT_EQ(1, compiler.chunk->code.size());
}

TEST(Compiler, endCompiler) {
  auto obj = new Obj{};
  auto compiler = Compiler("", new Chunk, new Table{}, &obj);
  compiler.endCompiler();
  EXPECT_EQ(1, compiler.chunk->code.size());
}

TEST(Compiler, advance) {
  auto obj = new Obj{};
  auto compiler = Compiler("this", new Chunk, new Table{}, &obj);
  compiler.advance();
  EXPECT_FALSE(compiler.parser.panicMode);

  compiler = Compiler("\"afa", new Chunk, new Table{}, &obj);
  compiler.advance();
  EXPECT_TRUE(compiler.parser.panicMode);
}

TEST(Compiler, consume) {
  auto obj = new Obj{};
  const char* msg = "error";
  auto compiler = Compiler("this", new Chunk, new Table{}, &obj);
  compiler.parser.current = compiler.scanner.scanToken();
  compiler.consume(TokenType::TOKEN_THIS, msg);
  EXPECT_FALSE(compiler.parser.panicMode);

  compiler = Compiler("thi ", new Chunk, new Table{}, &obj);
  compiler.parser.current = compiler.scanner.scanToken();
  compiler.consume(TokenType::TOKEN_THIS, msg);
  EXPECT_TRUE(compiler.parser.panicMode);
}

TEST(Compiler, errorAt) {
  auto obj = new Obj{};
  auto compiler = Compiler("this", new Chunk, new Table{}, &obj);
  compiler.errorAt(new Token{}, "msg");
  EXPECT_TRUE(compiler.parser.panicMode);
  EXPECT_TRUE(compiler.parser.hadError);
}

TEST(Compiler, emitByte) {
  auto obj = new Obj{};
  auto compiler = Compiler("this", new Chunk, new Table{}, &obj);
  compiler.parser.previous.line = 255;
  compiler.emitByte(255);

  EXPECT_EQ(compiler.chunk->code[0], 255);
  EXPECT_EQ(compiler.chunk->lines[0], 255);
}

TEST(Compiler, emitConstant) {
  auto obj = new Obj{};
  auto compiler = Compiler("this", new Chunk, new Table{}, &obj);
  Value value = NUMBER_VAL(1.1);
  compiler.parser.previous.line = 255;
  compiler.emitConstant(value);
  EXPECT_EQ(compiler.chunk->code[0], OptCode::OP_CONSTANT);
  EXPECT_EQ(compiler.chunk->code[1], 0);
  EXPECT_DOUBLE_EQ(compiler.chunk->constants.peek()->number, value.number);
}

TEST(Compiler, expressionStatement) {
  auto obj = new Obj{};
  auto compiler = Compiler("1.1;", new Chunk, new Table{}, &obj);
  compiler.advance();
  compiler.expressionStatement();
  ASSERT_EQ(compiler.chunk->constants.values.size(), 1);
  EXPECT_DOUBLE_EQ(compiler.chunk->constants.peek()->number, 1.1);
  ASSERT_EQ(compiler.chunk->code.size(), 3);
  EXPECT_EQ(compiler.chunk->code[0], OptCode::OP_CONSTANT);
  EXPECT_EQ(compiler.chunk->code[1], 0);
  EXPECT_EQ(compiler.chunk->code[2], OptCode::OP_POP);
}

TEST(Compiler, printStatement) {
  auto obj = new Obj{};
  auto compiler = Compiler("1.1;", new Chunk, new Table{}, &obj);
  compiler.advance();
  compiler.printStatement();
  ASSERT_EQ(compiler.chunk->constants.values.size(), 1);
  EXPECT_DOUBLE_EQ(compiler.chunk->constants.peek()->number, 1.1);
  ASSERT_EQ(compiler.chunk->code.size(), 3);
  EXPECT_EQ(compiler.chunk->code[0], OptCode::OP_CONSTANT);
  EXPECT_EQ(compiler.chunk->code[1], 0);
  EXPECT_EQ(compiler.chunk->code[2], OptCode::OP_PRINT);
}

TEST(Compiler, defineVariable) {
  {  // global variable
    auto obj = new Obj{};
    auto compiler = Compiler("", new Chunk, new Table{}, &obj);
    compiler.defineVariable(233);
    ASSERT_EQ(compiler.chunk->code.size(), 2);
    ASSERT_EQ(compiler.chunk->code[0], OptCode::OP_DEFINE_GLOBAL);
    ASSERT_EQ(compiler.chunk->code[1], 233);
  }
  {
    auto obj = new Obj{};
    auto compiler = Compiler("", new Chunk, new Table{}, &obj);
    compiler.scopeDepth = 100;
    compiler.localCount++;
    compiler.defineVariable(233);
    ASSERT_EQ(compiler.locals[0].depth, 100);
  }
}

TEST(Compiler, parseVariable) {
  auto obj = new Obj{};
  auto compiler = Compiler("abcd", new Chunk, new Table{}, &obj);
  compiler.advance();
  compiler.parseVariable("aaa");
  ASSERT_EQ(compiler.chunk->constants.values.size(), 1);
  ASSERT_EQ(((ObjString*)(compiler.chunk->constants.peek()->obj))->str, "abcd");
}

TEST(Compiler, identifierConstant) {
  auto obj = new Obj{};
  auto compiler = Compiler("abcd", new Chunk, new Table{}, &obj);
  Token token = compiler.scanner.scanToken();
  compiler.identifierConstant(&token);
  ASSERT_EQ(compiler.chunk->constants.values.size(), 1);
  ASSERT_EQ(((ObjString*)(compiler.chunk->constants.peek()->obj))->str, "abcd");
}

TEST(Compiler, makeConstant) {
  auto obj = new Obj{};
  auto compiler = Compiler("this", new Chunk, new Table{}, &obj);
  Value value = NUMBER_VAL(1.1);
  EXPECT_EQ(compiler.makeConstant(value), 0);
  EXPECT_DOUBLE_EQ(compiler.chunk->constants.peek()->number, value.number);
  EXPECT_EQ(compiler.chunk->constants.values.size(), 1);
}

TEST(Compiler, number) {
  auto obj = new Obj{};
  auto compiler = Compiler("1.1", new Chunk, new Table{}, &obj);
  compiler.advance(), compiler.advance();

  number(&compiler, false);
  EXPECT_EQ(compiler.chunk->constants.values.size(), 1);
  EXPECT_DOUBLE_EQ(compiler.chunk->constants.peek()->number, 1.1);
}

TEST(Compiler, grouping) {
  auto obj = new Obj{};
  auto compiler = new Compiler("(1+2)", new Chunk, new Table{}, &obj);
  compiler->advance();  // current on (
  grouping(compiler, false);
  EXPECT_EQ(compiler->chunk->constants.values.size(), 2);
  EXPECT_EQ(compiler->chunk->code.size(), 5);
  EXPECT_EQ(compiler->chunk->code[0], OptCode::OP_CONSTANT);
  EXPECT_EQ(compiler->chunk->code[1], 0);
  EXPECT_EQ(compiler->chunk->code[2], OptCode::OP_CONSTANT);
  EXPECT_EQ(compiler->chunk->code[3], 1);
  EXPECT_EQ(compiler->chunk->code[4], OptCode::OP_ADD);
}

TEST(Compiler, binary) {
#define run(op, opt_code, is_pair, second_opt_code)                          \
  {                                                                          \
    auto obj = new Obj{};                                                    \
    auto compiler = new Compiler("1" #op "2", new Chunk, new Table{}, &obj); \
    compiler->advance();                                                     \
    binary(compiler, false);                                                 \
    EXPECT_EQ(compiler->chunk->constants.values.size(), 2);                  \
    EXPECT_DOUBLE_EQ(compiler->chunk->constants.values[0].number, 1);        \
    EXPECT_DOUBLE_EQ(compiler->chunk->constants.values[1].number, 2);        \
    EXPECT_EQ(compiler->chunk->code.size(), is_pair ? 6 : 5);                \
    EXPECT_EQ(compiler->chunk->code[0], OptCode::OP_CONSTANT);               \
    EXPECT_EQ(compiler->chunk->code[1], 0);                                  \
    EXPECT_EQ(compiler->chunk->code[2], OptCode::OP_CONSTANT);               \
    EXPECT_EQ(compiler->chunk->code[3], 1);                                  \
    EXPECT_EQ(compiler->chunk->code[4], opt_code);                           \
    if (is_pair) {                                                           \
      EXPECT_EQ(compiler->chunk->code[5], second_opt_code);                  \
    };                                                                       \
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
    auto obj = new Obj{};
    auto compiler = new Compiler("-100", new Chunk, new Table{}, &obj);
    compiler->advance();  // previous on -
    unary(compiler, false);
    EXPECT_EQ(compiler->chunk->constants.values.size(), 1);
    EXPECT_DOUBLE_EQ(compiler->chunk->constants.peek()->number, 100);
    EXPECT_EQ(compiler->chunk->code.size(), 3);
    EXPECT_EQ(compiler->chunk->code[0], OptCode::OP_CONSTANT);
    EXPECT_EQ(compiler->chunk->code[1], 0);
    EXPECT_EQ(compiler->chunk->code[2], OptCode::OP_NEGATE);
  }
  {
    auto obj = new Obj{};
    auto compiler = new Compiler("!true", new Chunk, new Table{}, &obj);
    compiler->advance();  // previous on !
    unary(compiler, false);
    ASSERT_EQ(compiler->chunk->code.size(), 2);
    EXPECT_EQ(compiler->chunk->code[0], OptCode::OP_TRUE);
    EXPECT_EQ(compiler->chunk->code[1], OptCode::OP_NOT);
  }
}

TEST(Compiler, literal) {
  auto obj = new Obj{};
  auto compiler = new Compiler("false true nil", new Chunk, new Table{}, &obj);
  compiler->advance(), compiler->advance();
  literal(compiler, false);
  ASSERT_TRUE(compiler->chunk->code.size() > 0);
  EXPECT_EQ(compiler->chunk->code[0], OptCode::OP_FALSE);
  compiler->advance();
  literal(compiler, false);
  EXPECT_EQ(compiler->chunk->code[1], OptCode::OP_TRUE);
  compiler->advance();
  literal(compiler, false);
  EXPECT_EQ(compiler->chunk->code[2], OptCode::OP_NIL);
}

TEST(Compiler, namedVariable) {
  {
    auto obj = new Obj{};
    auto compiler = new Compiler("variable", new Chunk, new Table{}, &obj);
    compiler->advance();
    compiler->advance();
    compiler->namedVariable(compiler->parser.previous, false);
    ASSERT_EQ(compiler->chunk->constants.values.size(), 1);
    ASSERT_EQ(((ObjString*)(compiler->chunk->constants.peek()->obj))->str,
              "variable");
    ASSERT_EQ(compiler->chunk->code.size(), 2);
    ASSERT_EQ(compiler->chunk->code[0], OptCode::OP_GET_GLOBAL);
    ASSERT_EQ(compiler->chunk->code[1], 0);
  }
  {
    auto obj = new Obj{};
    auto compiler =
        new Compiler("variable = 1000.1", new Chunk, new Table{}, &obj);
    compiler->advance();
    compiler->advance();
    compiler->namedVariable(compiler->parser.previous, true);
    ASSERT_EQ(compiler->chunk->constants.values.size(), 2);
    ASSERT_EQ(((ObjString*)(compiler->chunk->constants.values[0].obj))->str,
              "variable");
    ASSERT_DOUBLE_EQ(compiler->chunk->constants.values[1].number, 1000.1);
    ASSERT_EQ(compiler->chunk->code.size(), 4);
    ASSERT_EQ(compiler->chunk->code[0], OptCode::OP_CONSTANT);
    ASSERT_EQ(compiler->chunk->code[1], 1);
    ASSERT_EQ(compiler->chunk->code[2], OptCode::OP_SET_GLOBAL);
    ASSERT_EQ(compiler->chunk->code[3], 0);
  }
}

TEST(Compiler, string) {
  auto obj = new Obj{};
  std::string raw = "\"this is string\"";
  auto compiler = new Compiler(raw.c_str(), new Chunk, new Table{}, &obj);
  compiler->advance(), compiler->advance();
  string(compiler, false);
  ASSERT_EQ(compiler->chunk->code.size(), 2);
  ASSERT_EQ(compiler->chunk->constants.values.size(), 1);
  EXPECT_EQ(compiler->chunk->code[0], OptCode::OP_CONSTANT);
  EXPECT_EQ(compiler->chunk->code[1], 0);

  auto val = compiler->chunk->constants.values[0];
  ASSERT_TRUE(IS_OBJ(val));
  EXPECT_EQ(AS_STRING(val)->str, "this is string");
}

TEST(Compiler, expression) {
  {
    auto obj = new Obj{};
    auto compiler = new Compiler("1+(2*3)", new Chunk, new Table{}, &obj);
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
    auto obj = new Obj{};
    auto compiler = new Compiler("1+(2*3-1.1)", new Chunk, new Table{}, &obj);
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
  auto obj = new Obj{};
  auto compiler = new Compiler("-1.1+1000", new Chunk, new Table{}, &obj);
  compiler->advance();  // current on -
  compiler->parsePrecedence(Precedence::PREC_TERM);
  EXPECT_EQ(compiler->chunk->constants.values.size(), 1);
  EXPECT_DOUBLE_EQ(compiler->chunk->constants.peek()->number, 1.1);

  EXPECT_EQ(compiler->chunk->code.size(), 3);
  EXPECT_EQ(compiler->chunk->code[0], OptCode::OP_CONSTANT);
  EXPECT_EQ(compiler->chunk->code[1], 0);
  EXPECT_EQ(compiler->chunk->code[2], OptCode::OP_NEGATE);
}

TEST(Compiler, addLocal) {
  auto obj = new Obj{};
  auto compiler = new Compiler("", new Chunk, new Table{}, &obj);
  Token token;

  // too many
  compiler->localCount = UINT8_COUNT;
  compiler->addLocal(token);
  ASSERT_EQ(compiler->localCount, UINT8_COUNT);

  compiler->localCount = 10;
  token.line = 100;
  compiler->addLocal(token);
  ASSERT_EQ(compiler->locals[10].depth, -1);  // uninitialized
  ASSERT_EQ(compiler->locals[10].name.line, 100);
}

TEST(Compiler, declareVariable) {
  auto obj = new Obj{};
  auto compiler = new Compiler("a a", new Chunk, new Table{}, &obj);
  compiler->advance(), compiler->advance();  // previous on a
  compiler->localCount = 1;
  compiler->locals[0].depth = 1;
  compiler->declareVariable();

  ASSERT_EQ(compiler->localCount, 2);
  EXPECT_EQ(*(compiler->locals[1].name.start), 'a');

  compiler->advance();
  compiler->declareVariable();  // already defined
  EXPECT_TRUE(compiler->parser.panicMode);
}

TEST(Compiler, beginScope) {
  auto obj = new Obj{};
  auto compiler = new Compiler("", new Chunk, new Table{}, &obj);
  compiler->beginScope();
  EXPECT_EQ(compiler->scopeDepth, 1);
}

TEST(Compiler, endScope) {
  auto obj = new Obj{};
  auto compiler = new Compiler("a a", new Chunk, new Table{}, &obj);
  compiler->scopeDepth = 10;

  auto count = 100;
  compiler->localCount = count;
  for (auto i = 0; i < count; i++) compiler->locals[i].depth = 100;
  compiler->endScope();
  ASSERT_EQ(compiler->scopeDepth, 9);
  ASSERT_EQ(compiler->chunk->code.size(), count);
  for (auto i = 0; i < count; i++)
    EXPECT_EQ(compiler->chunk->code[i], OptCode::OP_POP);
}

TEST(Compiler, resolveLocal) {
  auto obj = new Obj{};
  auto compiler = new Compiler("a a", new Chunk, new Table{}, &obj);
  compiler->advance(), compiler->advance();  // previous on a
  compiler->localCount = 1;
  compiler->locals[0].depth = 1;
  compiler->scopeDepth = 2;
  compiler->declareVariable();

  ASSERT_EQ(compiler->resolveLocal(&compiler->parser.current), 1);
}
