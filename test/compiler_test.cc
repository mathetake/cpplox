#include "main/compiler.hpp"

#include <gtest/gtest.h>

#include "main/value.hpp"

Obj* tmpObj = new Obj{};

#define NEW_COMPILER(source) new Compiler(source, new Table{}, &tmpObj)

TEST(Compiler, check) {
  auto compiler = NEW_COMPILER("true");
  compiler->advance();
  ASSERT_TRUE(compiler->check(TokenType::TOKEN_TRUE));
  ASSERT_FALSE(compiler->check(TokenType::TOKEN_FALSE));
}

TEST(Compiler, match) {
  auto compiler = NEW_COMPILER("1011");
  compiler->advance();
  ASSERT_TRUE(compiler->match(TokenType::TOKEN_NUMBER));
  ASSERT_TRUE(compiler->match(TokenType::TOKEN_EOF));
}

TEST(Compiler, Constructor) {
  auto src = "abcde";
  auto obj = new Obj{};
  auto strTable = new Table{};
  auto compiler = new Compiler(src, strTable, &obj);

  EXPECT_EQ(compiler->stringTable, strTable);
  EXPECT_EQ(compiler->objects, &obj);
  EXPECT_EQ(compiler->scanner->start, src);
}

TEST(Compiler, emitReturn) {
  auto compiler = NEW_COMPILER("");
  compiler->emitReturn();
  EXPECT_EQ(2, compiler->function->chunk.code.size());
}

TEST(Compiler, endCompiler) {
  auto compiler = NEW_COMPILER("");
  compiler->endCompiler();
  EXPECT_EQ(2, compiler->function->chunk.code.size());
}

TEST(Compiler, advance) {
  auto compiler = NEW_COMPILER("this");
  compiler->advance();
  EXPECT_FALSE(compiler->parser->panicMode);

  compiler = NEW_COMPILER("\"afa");
  compiler->advance();
  EXPECT_TRUE(compiler->parser->panicMode);
}

TEST(Compiler, consume) {
  const char* msg = "error";
  auto compiler = NEW_COMPILER("this");
  compiler->parser->current = compiler->scanner->scanToken();
  compiler->consume(TokenType::TOKEN_THIS, msg);
  EXPECT_FALSE(compiler->parser->panicMode);

  compiler = NEW_COMPILER("thi ");
  compiler->parser->current = compiler->scanner->scanToken();
  compiler->consume(TokenType::TOKEN_THIS, msg);
  EXPECT_TRUE(compiler->parser->panicMode);
}

TEST(Compiler, errorAt) {
  auto compiler = NEW_COMPILER("this");
  compiler->errorAt(new Token{}, "msg");
  EXPECT_TRUE(compiler->parser->panicMode);
  EXPECT_TRUE(compiler->parser->hadError);
}

TEST(Compiler, emitByte) {
  auto compiler = NEW_COMPILER("this");
  compiler->parser->previous.line = 255;
  compiler->emitByte(255);

  EXPECT_EQ(compiler->function->chunk.code[0], 255);
  EXPECT_EQ(compiler->function->chunk.lines[0], 255);
}

TEST(Compiler, emitConstant) {
  auto compiler = NEW_COMPILER("this");
  Value value = NUMBER_VAL(1.1);
  compiler->parser->previous.line = 255;
  compiler->emitConstant(value);
  EXPECT_EQ(compiler->function->chunk.code[0], OptCode::OP_CONSTANT);
  EXPECT_EQ(compiler->function->chunk.code[1], 0);
  EXPECT_DOUBLE_EQ(compiler->function->chunk.constants.peek()->number,
                   value.number);
}

TEST(Compiler, expressionStatement) {
  auto compiler = NEW_COMPILER("1.1;");
  compiler->advance();
  compiler->expressionStatement();
  ASSERT_EQ(compiler->function->chunk.constants.values.size(), 1);
  EXPECT_DOUBLE_EQ(compiler->function->chunk.constants.peek()->number, 1.1);
  ASSERT_EQ(compiler->function->chunk.code.size(), 3);
  EXPECT_EQ(compiler->function->chunk.code[0], OptCode::OP_CONSTANT);
  EXPECT_EQ(compiler->function->chunk.code[1], 0);
  EXPECT_EQ(compiler->function->chunk.code[2], OptCode::OP_POP);
}

TEST(Compiler, printStatement) {
  auto compiler = NEW_COMPILER("1.1;");
  compiler->advance();
  compiler->printStatement();
  ASSERT_EQ(compiler->function->chunk.constants.values.size(), 1);
  EXPECT_DOUBLE_EQ(compiler->function->chunk.constants.peek()->number, 1.1);
  ASSERT_EQ(compiler->function->chunk.code.size(), 3);
  EXPECT_EQ(compiler->function->chunk.code[0], OptCode::OP_CONSTANT);
  EXPECT_EQ(compiler->function->chunk.code[1], 0);
  EXPECT_EQ(compiler->function->chunk.code[2], OptCode::OP_PRINT);
}

TEST(Compiler, defineVariable) {
  {  // global variable
    auto compiler = NEW_COMPILER("");
    compiler->defineVariable(233);
    ASSERT_EQ(compiler->function->chunk.code.size(), 2);
    ASSERT_EQ(compiler->function->chunk.code[0], OptCode::OP_DEFINE_GLOBAL);
    ASSERT_EQ(compiler->function->chunk.code[1], 233);
  }
  {
    auto compiler = NEW_COMPILER("");
    compiler->scopeDepth = 100;
    compiler->localCount++;
    compiler->defineVariable(233);
    ASSERT_EQ(compiler->locals[1].depth, 100);
  }
}

TEST(Compiler, parseVariable) {
  auto compiler = NEW_COMPILER("abcd");
  compiler->advance();
  compiler->parseVariable("aaa");
  ASSERT_EQ(compiler->function->chunk.constants.values.size(), 1);
  ASSERT_EQ(
      ((ObjString*)(compiler->function->chunk.constants.peek()->obj))->str,
      "abcd");
}

TEST(Compiler, identifierConstant) {
  auto compiler = NEW_COMPILER("abcd");
  Token token = compiler->scanner->scanToken();
  compiler->identifierConstant(&token);
  ASSERT_EQ(compiler->function->chunk.constants.values.size(), 1);
  ASSERT_EQ(
      ((ObjString*)(compiler->function->chunk.constants.peek()->obj))->str,
      "abcd");
}

TEST(Compiler, makeConstant) {
  auto compiler = NEW_COMPILER("this");
  Value value = NUMBER_VAL(1.1);
  EXPECT_EQ(compiler->makeConstant(value), 0);
  EXPECT_DOUBLE_EQ(compiler->function->chunk.constants.peek()->number,
                   value.number);
  EXPECT_EQ(compiler->function->chunk.constants.values.size(), 1);
}

TEST(Compiler, number) {
  auto compiler = NEW_COMPILER("1.1");
  compiler->advance(), compiler->advance();

  number(compiler, false);
  EXPECT_EQ(compiler->function->chunk.constants.values.size(), 1);
  EXPECT_DOUBLE_EQ(compiler->function->chunk.constants.peek()->number, 1.1);
}

TEST(Compiler, grouping) {
  auto compiler = NEW_COMPILER("(1+2)");
  compiler->advance();  // current on (
  grouping(compiler, false);
  EXPECT_EQ(compiler->function->chunk.constants.values.size(), 2);
  EXPECT_EQ(compiler->function->chunk.code.size(), 5);
  EXPECT_EQ(compiler->function->chunk.code[0], OptCode::OP_CONSTANT);
  EXPECT_EQ(compiler->function->chunk.code[1], 0);
  EXPECT_EQ(compiler->function->chunk.code[2], OptCode::OP_CONSTANT);
  EXPECT_EQ(compiler->function->chunk.code[3], 1);
  EXPECT_EQ(compiler->function->chunk.code[4], OptCode::OP_ADD);
}

TEST(Compiler, binary) {
#define run(op, opt_code, is_pair, second_opt_code)                            \
  {                                                                            \
    auto compiler = NEW_COMPILER("1" #op "2");                                 \
    compiler->advance();                                                       \
    compiler->expression();                                                    \
    EXPECT_EQ(compiler->function->chunk.constants.values.size(), 2);           \
    EXPECT_DOUBLE_EQ(compiler->function->chunk.constants.values[0].number, 1); \
    EXPECT_DOUBLE_EQ(compiler->function->chunk.constants.values[1].number, 2); \
    EXPECT_EQ(compiler->function->chunk.code.size(), is_pair ? 6 : 5);         \
    EXPECT_EQ(compiler->function->chunk.code[0], OptCode::OP_CONSTANT);        \
    EXPECT_EQ(compiler->function->chunk.code[1], 0);                           \
    EXPECT_EQ(compiler->function->chunk.code[2], OptCode::OP_CONSTANT);        \
    EXPECT_EQ(compiler->function->chunk.code[3], 1);                           \
    EXPECT_EQ(compiler->function->chunk.code[4], opt_code);                    \
    if (is_pair) {                                                             \
      EXPECT_EQ(compiler->function->chunk.code[5], second_opt_code);           \
    };                                                                         \
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
    auto compiler = NEW_COMPILER("-100");
    compiler->advance();  // previous on -
    unary(compiler, false);
    EXPECT_EQ(compiler->function->chunk.constants.values.size(), 1);
    EXPECT_DOUBLE_EQ(compiler->function->chunk.constants.peek()->number, 100);
    EXPECT_EQ(compiler->function->chunk.code.size(), 3);
    EXPECT_EQ(compiler->function->chunk.code[0], OptCode::OP_CONSTANT);
    EXPECT_EQ(compiler->function->chunk.code[1], 0);
    EXPECT_EQ(compiler->function->chunk.code[2], OptCode::OP_NEGATE);
  }
  {
    auto compiler = NEW_COMPILER("!true");
    compiler->advance();  // previous on !
    unary(compiler, false);
    ASSERT_EQ(compiler->function->chunk.code.size(), 2);
    EXPECT_EQ(compiler->function->chunk.code[0], OptCode::OP_TRUE);
    EXPECT_EQ(compiler->function->chunk.code[1], OptCode::OP_NOT);
  }
}

TEST(Compiler, literal) {
  auto compiler = NEW_COMPILER("false true nil");
  compiler->advance(), compiler->advance();
  literal(compiler, false);
  ASSERT_TRUE(compiler->function->chunk.code.size() > 0);
  EXPECT_EQ(compiler->function->chunk.code[0], OptCode::OP_FALSE);
  compiler->advance();
  literal(compiler, false);
  EXPECT_EQ(compiler->function->chunk.code[1], OptCode::OP_TRUE);
  compiler->advance();
  literal(compiler, false);
  EXPECT_EQ(compiler->function->chunk.code[2], OptCode::OP_NIL);
}

TEST(Compiler, namedVariable) {
  {
    auto compiler = NEW_COMPILER("variable");
    compiler->advance();
    compiler->advance();
    compiler->namedVariable(compiler->parser->previous, false);
    ASSERT_EQ(compiler->function->chunk.constants.values.size(), 1);
    ASSERT_EQ(
        ((ObjString*)(compiler->function->chunk.constants.peek()->obj))->str,
        "variable");
    ASSERT_EQ(compiler->function->chunk.code.size(), 2);
    ASSERT_EQ(compiler->function->chunk.code[0], OptCode::OP_GET_GLOBAL);
    ASSERT_EQ(compiler->function->chunk.code[1], 0);
  }
  {
    auto compiler = NEW_COMPILER("variable = 1000.1");
    compiler->advance();
    compiler->advance();
    compiler->namedVariable(compiler->parser->previous, true);
    ASSERT_EQ(compiler->function->chunk.constants.values.size(), 2);
    ASSERT_EQ(
        ((ObjString*)(compiler->function->chunk.constants.values[0].obj))->str,
        "variable");
    ASSERT_DOUBLE_EQ(compiler->function->chunk.constants.values[1].number,
                     1000.1);
    ASSERT_EQ(compiler->function->chunk.code.size(), 4);
    ASSERT_EQ(compiler->function->chunk.code[0], OptCode::OP_CONSTANT);
    ASSERT_EQ(compiler->function->chunk.code[1], 1);
    ASSERT_EQ(compiler->function->chunk.code[2], OptCode::OP_SET_GLOBAL);
    ASSERT_EQ(compiler->function->chunk.code[3], 0);
  }
}

TEST(Compiler, string) {
  auto compiler = NEW_COMPILER("\"this is string\"");
  compiler->advance(), compiler->advance();
  string(compiler, false);
  ASSERT_EQ(compiler->function->chunk.code.size(), 2);
  ASSERT_EQ(compiler->function->chunk.constants.values.size(), 1);
  EXPECT_EQ(compiler->function->chunk.code[0], OptCode::OP_CONSTANT);
  EXPECT_EQ(compiler->function->chunk.code[1], 0);

  auto val = compiler->function->chunk.constants.values[0];
  ASSERT_TRUE(IS_OBJ(val));
  EXPECT_EQ(AS_STRING(val)->str, "this is string");
}

TEST(Compiler, expression) {
  {
    auto compiler = NEW_COMPILER("1+(2*3)");
    compiler->advance();  // current on 1
    compiler->expression();
    EXPECT_EQ(compiler->function->chunk.constants.values.size(), 3);
    EXPECT_DOUBLE_EQ(compiler->function->chunk.constants.values[0].number, 1);
    EXPECT_DOUBLE_EQ(compiler->function->chunk.constants.values[1].number, 2);
    EXPECT_DOUBLE_EQ(compiler->function->chunk.constants.values[2].number, 3);

    EXPECT_EQ(compiler->function->chunk.code.size(), 8);
    EXPECT_EQ(compiler->function->chunk.code[0], OptCode::OP_CONSTANT);
    EXPECT_EQ(compiler->function->chunk.code[1], 0);

    EXPECT_EQ(compiler->function->chunk.code[2], OptCode::OP_CONSTANT);
    EXPECT_EQ(compiler->function->chunk.code[3], 1);

    EXPECT_EQ(compiler->function->chunk.code[4], OptCode::OP_CONSTANT);
    EXPECT_EQ(compiler->function->chunk.code[5], 2);

    EXPECT_EQ(compiler->function->chunk.code[6], OptCode::OP_MULTIPLY);
    EXPECT_EQ(compiler->function->chunk.code[7], OptCode::OP_ADD);
  }
  {
    auto compiler = NEW_COMPILER("1+(2*3-1.1)");
    compiler->advance();  // current on 1
    compiler->expression();
    EXPECT_EQ(compiler->function->chunk.constants.values.size(), 4);
    EXPECT_DOUBLE_EQ(compiler->function->chunk.constants.values[0].number, 1);
    EXPECT_DOUBLE_EQ(compiler->function->chunk.constants.values[1].number, 2);
    EXPECT_DOUBLE_EQ(compiler->function->chunk.constants.values[2].number, 3);
    EXPECT_DOUBLE_EQ(compiler->function->chunk.constants.values[3].number, 1.1);

    EXPECT_EQ(compiler->function->chunk.code.size(), 11);
    EXPECT_EQ(compiler->function->chunk.code[0], OptCode::OP_CONSTANT);
    EXPECT_EQ(compiler->function->chunk.code[1], 0);

    EXPECT_EQ(compiler->function->chunk.code[2], OptCode::OP_CONSTANT);
    EXPECT_EQ(compiler->function->chunk.code[3], 1);

    EXPECT_EQ(compiler->function->chunk.code[4], OptCode::OP_CONSTANT);
    EXPECT_EQ(compiler->function->chunk.code[5], 2);

    EXPECT_EQ(compiler->function->chunk.code[6], OptCode::OP_MULTIPLY);

    EXPECT_EQ(compiler->function->chunk.code[7], OptCode::OP_CONSTANT);
    EXPECT_EQ(compiler->function->chunk.code[8], 3);
    EXPECT_EQ(compiler->function->chunk.code[9], OptCode::OP_SUBTRACT);
    EXPECT_EQ(compiler->function->chunk.code[10], OptCode::OP_ADD);
  }
}

TEST(Compiler, parsePrecedence) {
  auto compiler = NEW_COMPILER("-1.1+1000");
  compiler->advance();  // current on -
  compiler->parsePrecedence(Precedence::PREC_TERM);
  EXPECT_EQ(compiler->function->chunk.constants.values.size(), 1);
  EXPECT_DOUBLE_EQ(compiler->function->chunk.constants.peek()->number, 1.1);

  EXPECT_EQ(compiler->function->chunk.code.size(), 3);
  EXPECT_EQ(compiler->function->chunk.code[0], OptCode::OP_CONSTANT);
  EXPECT_EQ(compiler->function->chunk.code[1], 0);
  EXPECT_EQ(compiler->function->chunk.code[2], OptCode::OP_NEGATE);
}

TEST(Compiler, addLocal) {
  auto compiler = NEW_COMPILER("");
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
  auto compiler = NEW_COMPILER("a a");
  compiler->advance(), compiler->advance();  // previous on a
  compiler->localCount = 1;
  compiler->locals[0].depth = 1;
  compiler->declareVariable();

  ASSERT_EQ(compiler->localCount, 2);
  EXPECT_EQ(*(compiler->locals[1].name.start), 'a');

  compiler->advance();
  compiler->declareVariable();  // already defined
  EXPECT_TRUE(compiler->parser->panicMode);
}

TEST(Compiler, beginScope) {
  auto compiler = NEW_COMPILER("");
  compiler->beginScope();
  EXPECT_EQ(compiler->scopeDepth, 1);
}

TEST(Compiler, endScope) {
  auto compiler = NEW_COMPILER("a a");
  compiler->scopeDepth = 10;

  auto count = 100;
  compiler->localCount = count;
  for (auto i = 0; i < count; i++) compiler->locals[i].depth = 100;
  compiler->endScope();
  ASSERT_EQ(compiler->scopeDepth, 9);
  ASSERT_EQ(compiler->function->chunk.code.size(), count);
  for (auto i = 0; i < count; i++)
    EXPECT_EQ(compiler->function->chunk.code[i], OptCode::OP_POP);
}

TEST(Compiler, resolveLocal) {
  auto compiler = NEW_COMPILER("a a");
  compiler->advance(), compiler->advance();  // previous on a
  compiler->localCount = 1;
  compiler->locals[0].depth = 1;
  compiler->scopeDepth = 2;
  compiler->declareVariable();

  ASSERT_EQ(compiler->resolveLocal(&compiler->parser->current), 1);
}

TEST(Compiler, emitJump) {
  auto compiler = NEW_COMPILER("");
  ASSERT_EQ(compiler->emitJump(OptCode::OP_JUMP_IF_FALSE), 1);
  ASSERT_EQ(compiler->function->chunk.code[0], OptCode::OP_JUMP_IF_FALSE);
  ASSERT_EQ(compiler->function->chunk.code[1], 0xff);
  ASSERT_EQ(compiler->function->chunk.code[2], 0xff);
}

TEST(Compiler, patchJump) {
  auto obj = new Obj{};
  auto compiler = new Compiler("", new Table{}, &obj);

  compiler->function->chunk.code.push_back(OptCode::OP_JUMP_IF_FALSE);
  compiler->function->chunk.code.push_back(0xf1);
  compiler->function->chunk.code.push_back(0xf2);
  compiler->function->chunk.code.push_back(0x03);
  ASSERT_EQ(compiler->function->chunk.count(), 4);
  compiler->patchJump(1);
  ASSERT_EQ(compiler->function->chunk.code[1], 0x00);
  ASSERT_EQ(compiler->function->chunk.code[2], 0x01);
}

TEST(Compiler, andOp) {
  auto compiler = NEW_COMPILER("true");
  compiler->advance();  // curren on true
  andOp(compiler, false);

  ASSERT_EQ(compiler->function->chunk.code.size(), 5);
  EXPECT_EQ(compiler->function->chunk.code[0], OptCode::OP_JUMP_IF_FALSE);
  EXPECT_EQ(compiler->function->chunk.code[1], 0x00);
  EXPECT_EQ(compiler->function->chunk.code[2], 0x02);
  EXPECT_EQ(compiler->function->chunk.code[3], OptCode::OP_POP);
  EXPECT_EQ(compiler->function->chunk.code[4], OptCode::OP_TRUE);
}

TEST(Compiler, orOp) {
  auto compiler = NEW_COMPILER("true");
  compiler->advance();  // curren on true
  orOp(compiler, false);
  ASSERT_EQ(compiler->function->chunk.code.size(), 8);
  EXPECT_EQ(compiler->function->chunk.code[0], OptCode::OP_JUMP_IF_FALSE);
  EXPECT_EQ(compiler->function->chunk.code[1], 0x00);
  EXPECT_EQ(compiler->function->chunk.code[2], 0x03);
  EXPECT_EQ(compiler->function->chunk.code[3], OptCode::OP_JUMP);
  EXPECT_EQ(compiler->function->chunk.code[4], 0x00);
  EXPECT_EQ(compiler->function->chunk.code[5], 0x02);
  EXPECT_EQ(compiler->function->chunk.code[6], OptCode::OP_POP);
  EXPECT_EQ(compiler->function->chunk.code[7], OptCode::OP_TRUE);
}

TEST(Compiler, emitLoop) {
  auto compiler = NEW_COMPILER("");
  for (auto i = 0; i < 10; i++) compiler->emitByte(0x01);
  compiler->emitLoop(0);
  ASSERT_EQ(compiler->function->chunk.code[10], OptCode::OP_LOOP);
  ASSERT_EQ(compiler->function->chunk.code[11], 0x00);
  ASSERT_EQ(compiler->function->chunk.code[12], 0x0d);
}

TEST(Compiler, compileFunction) {
  auto compiler = NEW_COMPILER("name () { 100;}");
  compiler->advance(), compiler->advance();
  compiler->compileFunction(FunctionType::TYPE_FUNCTION);
  ASSERT_EQ(compiler->function->chunk.code.size(), 2);

  ASSERT_EQ(compiler->function->chunk.code[0], OptCode::OP_CLOSURE);
  ASSERT_EQ(compiler->function->chunk.code[1], 0);

  ASSERT_EQ(compiler->function->chunk.constants.values.size(), 1);
  auto value = compiler->function->chunk.constants.values[0];
  ASSERT_TRUE(IS_OBJ(value));
  ObjFunction* function = AS_FUNCTION(value);
  ASSERT_TRUE(function);

  ASSERT_EQ(function->name->str, "name");
  ASSERT_EQ(function->chunk.code.size(), 5);
  ASSERT_EQ(function->chunk.code[0], OptCode::OP_CONSTANT);
  ASSERT_EQ(function->chunk.code[1], 0);
  ASSERT_EQ(function->chunk.constants.values.size(), 1);
  ASSERT_EQ(function->chunk.constants.values[0].number, 100);
}

TEST(Compiler, argumentList) {
#define run(code, exp)                        \
  {                                           \
    auto compiler = NEW_COMPILER(code);       \
    compiler->advance(), compiler->advance(); \
    ASSERT_EQ(compiler->argumentList(), exp); \
  }
  run("()", 0);
  run("(1)", 1);
  run("(1,2,3)", 3);
#undef run
}

TEST(Compiler, call) {
  auto compiler = NEW_COMPILER("()");
  compiler->advance(), compiler->advance();
  call(compiler, false);
  ASSERT_EQ(compiler->function->chunk.code.size(), 2);
  ASSERT_EQ(compiler->function->chunk.code[0], OptCode::OP_CALL);
  ASSERT_EQ(compiler->function->chunk.code[1], 0);
}
