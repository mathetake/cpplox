#include "main/scanner.hpp"

#include <gtest/gtest.h>

TEST(Scanner, skipWhitespace) {
#define run(src, expline)        \
  {                              \
    const char* source = src;    \
    auto sc = Scanner(source);   \
    sc.skipWhitespace();         \
    EXPECT_NE(sc.peek(), ' ');   \
    EXPECT_NE(sc.peek(), '\r');  \
    EXPECT_NE(sc.peek(), '\t');  \
    EXPECT_EQ(sc.line, expline); \
  }

  run("\t\ra", 1);
  run("\r\r vsavsa", 1);
  run("\t \r\n\nvsavsa", 3);
  run("\nvsavsa", 2);
  run("   //asdfasdfa", 1);
  run("/1", 1);
#undef run
}

TEST(Scanner, peek) {
  const char* source = "ab";
  auto sc = Scanner(source);
  EXPECT_EQ(sc.peek(), 'a');
  EXPECT_EQ(sc.peekNext(), 'b');
  sc.advance();
  EXPECT_EQ(sc.peek(), 'b');
  EXPECT_EQ(sc.peekNext(), '\0');
}

TEST(Scanner, isAtEnd) {
  const char* source = "a";
  auto sc = Scanner(source);
  EXPECT_FALSE(sc.isAtEnd());
  sc.advance();
  EXPECT_TRUE(sc.isAtEnd());
}
TEST(Scanner, number) {
  {  // no dot
    const char* source = "12345";
    auto actual = Scanner(source).number();
    EXPECT_EQ(actual.type, TokenType::TOKEN_NUMBER);
    EXPECT_EQ(actual.start, source);
    EXPECT_EQ(actual.length, 5);
  }
}
TEST(Scanner, identifier) {
  {  // identifier
    const char* source = "abcde";
    auto sc = Scanner(source);
    sc.advance();
    auto actual = sc.identifier();
    EXPECT_EQ(actual.type, TokenType::TOKEN_IDENTIFIER);
    EXPECT_EQ(actual.start, source);
    EXPECT_EQ(actual.length, 5);
  }
  {  // keyword
    const char* source = "return";
    auto sc = Scanner(source);
    sc.advance();
    auto actual = sc.identifier();
    EXPECT_EQ(actual.type, TokenType::TOKEN_RETURN);
    EXPECT_EQ(actual.start, source);
    EXPECT_EQ(actual.length, 6);
  }
}

TEST(Scanner, identifierType) {
#define run(src, advanced, exp)          \
  {                                      \
    const char* source = src;            \
    auto sc = Scanner(source);           \
    sc.current += advanced;              \
    EXPECT_EQ(sc.identifierType(), exp); \
  }

  run("an ", 2, TokenType::TOKEN_IDENTIFIER);
  run("and ", 3, TokenType::TOKEN_AND);
  run("ande", 4, TokenType::TOKEN_IDENTIFIER);
  run("class", 5, TokenType::TOKEN_CLASS);
  run("classe", 6, TokenType::TOKEN_IDENTIFIER);
  run("else", 4, TokenType::TOKEN_ELSE);
  run("el", 2, TokenType::TOKEN_IDENTIFIER);
  run("false", 5, TokenType::TOKEN_FALSE);
  run("for", 3, TokenType::TOKEN_FOR);
  run("fun", 3, TokenType::TOKEN_FUN);
  run("funn", 4, TokenType::TOKEN_IDENTIFIER);
  run("if", 2, TokenType::TOKEN_IF);
  run("i", 1, TokenType::TOKEN_IDENTIFIER);
  run("nil", 3, TokenType::TOKEN_NIL);
  run("nill", 4, TokenType::TOKEN_IDENTIFIER);
  run("or", 2, TokenType::TOKEN_OR);
  run("o", 1, TokenType::TOKEN_IDENTIFIER);
  run("orr", 1, TokenType::TOKEN_IDENTIFIER);
  run("return", 6, TokenType::TOKEN_RETURN);
  run("retur", 5, TokenType::TOKEN_IDENTIFIER);
  run("print", 5, TokenType::TOKEN_PRINT);
  run("printt", 6, TokenType::TOKEN_IDENTIFIER);
  run("super", 5, TokenType::TOKEN_SUPER);
  run("sup", 3, TokenType::TOKEN_IDENTIFIER);
  run("this", 4, TokenType::TOKEN_THIS);
  run("thiss", 5, TokenType::TOKEN_IDENTIFIER);
  run("true", 4, TokenType::TOKEN_TRUE);
  run("tru", 3, TokenType::TOKEN_IDENTIFIER);
  run("var", 3, TokenType::TOKEN_VAR);
  run("varr", 4, TokenType::TOKEN_IDENTIFIER);
  run("while", 5, TokenType::TOKEN_WHILE);
  run("whilee", 6, TokenType::TOKEN_IDENTIFIER);
#undef run
}

TEST(Scanner, checkKeyword) {
  {
    const char* source = "and";
    auto sc = Scanner(source);
    sc.advance(), sc.advance(), sc.advance();
    EXPECT_EQ(sc.checkKeyword(1, 2, "nd", TokenType::TOKEN_AND),
              TokenType::TOKEN_AND);
  }
  {
    const char* source = "ane";
    auto sc = Scanner(source);
    sc.advance(), sc.advance(), sc.advance();
    EXPECT_EQ(sc.checkKeyword(1, 2, "nd", TokenType::TOKEN_AND),
              TokenType::TOKEN_IDENTIFIER);
  }
}

TEST(Scanner, stringLiteral) {
  {  // error
    const char* source = "a";
    auto sc = Scanner(source);
    EXPECT_EQ(sc.stringLiteral().type, TokenType::TOKEN_ERROR);
  }
  {  // ok
    const char* source = "aaa\"";
    auto sc = Scanner(source);
    auto actual = sc.stringLiteral();
    EXPECT_EQ(actual.type, TokenType::TOKEN_STRING);
    EXPECT_EQ(actual.start, source);
    EXPECT_EQ(actual.length, 4);
  }
}

TEST(Scanner, match) {
#define run(src, in, exp)                  \
  {                                        \
    const char* source = src;              \
    auto sc = Scanner(source);             \
    EXPECT_EQ(sc.match(in), exp);          \
    if (exp) {                             \
      EXPECT_EQ(sc.current - sc.start, 1); \
    }                                      \
  }

  run("", 'a', false);   // at end
  run("a", 'a', true);   // eq
  run("a", 'b', false);  // neq
#undef run
}

TEST(Scanner, makeToken) {
  const char* source = "source";
  auto sc = Scanner(source);
  sc.current++;
  sc.line = 10;
  auto actual = sc.makeToken(TokenType::TOKEN_FALSE);

  EXPECT_EQ(actual.length, 1);
  EXPECT_EQ(actual.line, 10);
  EXPECT_EQ(actual.start, source);
  EXPECT_EQ(actual.type, TokenType::TOKEN_FALSE);
}

TEST(Scanner, errorToken) {
  const char* msg = "error";
  auto sc = Scanner(nullptr);
  sc.line = 1000;
  auto actual = sc.errorToken(msg);

  EXPECT_EQ(actual.length, 5);
  EXPECT_EQ(actual.line, 1000);
  EXPECT_EQ(actual.start, msg);
  EXPECT_EQ(actual.type, TokenType::TOKEN_ERROR);
}

TEST(Scanner, advance) {
  const char* source = "ab";
  auto sc = Scanner(source);
  auto actual = sc.advance();

  EXPECT_EQ('a', actual);
  EXPECT_EQ(*sc.current, 'b');
}

TEST(Scanner, scanToken) {
  {  // number
    EXPECT_EQ(Scanner("123131").scanToken().type, TokenType::TOKEN_NUMBER);
  }
  {  // identifier
    EXPECT_EQ(Scanner("return").scanToken().type, TokenType::TOKEN_RETURN);
    EXPECT_EQ(Scanner("abc").scanToken().type, TokenType::TOKEN_IDENTIFIER);
  }

#define run_switch(src, exptype)     \
  {                                  \
    const char* source = src;        \
    Scanner sc = Scanner(source);    \
    Token actual = sc.scanToken();   \
    EXPECT_EQ(actual.type, exptype); \
  };

  run_switch("", TokenType::TOKEN_EOF);
  run_switch("  \t(", TokenType::TOKEN_LEFT_PAREN);
  run_switch(")", TokenType::TOKEN_RIGHT_PAREN);
  run_switch(" \n {", TokenType::TOKEN_LEFT_BRACE);
  run_switch("}", TokenType::TOKEN_RIGHT_BRACE);
  run_switch(" ;", TokenType::TOKEN_SEMICOLON);
  run_switch("\r,", TokenType::TOKEN_COMMA);
  run_switch(".", TokenType::TOKEN_DOT);
  run_switch("-", TokenType::TOKEN_MINUS);
  run_switch("+", TokenType::TOKEN_PLUS);
  run_switch("/", TokenType::TOKEN_SLASH);
  run_switch("*", TokenType::TOKEN_STAR);

  run_switch("!=", TokenType::TOKEN_BANG_EQUAL);
  run_switch("!", TokenType::TOKEN_BANG);
  run_switch("==", TokenType::TOKEN_EQUAL_EQUAL);
  run_switch("=", TokenType::TOKEN_EQUAL);
  run_switch("\n \r  <=", TokenType::TOKEN_LESS_EQUAL);
  run_switch("<", TokenType::TOKEN_LESS);
  run_switch(">=", TokenType::TOKEN_GREATER_EQUAL);
  run_switch(">", TokenType::TOKEN_GREATER);
#undef run_switch
}