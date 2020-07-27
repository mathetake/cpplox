#ifndef cpplox_compiler_h
#define cpplox_compiler_h
#include "chunk.hpp"
#include "common.hpp"
#include "scanner.hpp"

enum Precedence {
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
  PREC_CALL,        // . ()
  PREC_PRIMARY
};

class Parser {
 public:
  Token current;
  Token previous;
  bool hadError;
  bool panicMode;
};

class Compiler {
 public:
  Scanner scanner;
  Parser parser;
  Chunk* chunk;
  Compiler(const char* source, Chunk* targetChunk);

  bool compile();
  void advance();
  void consume(TokenType type, const char* message);
  void emitByte(uint8_t byte);
  void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1), emitByte(byte2);
  };
  void endCompiler();
  void emitReturn();
  void emitConstant(Value value);
  uint8_t makeConstant(Value value);

  void parsePrecedence(Precedence precedence);
  void expression();

  // errors
  void errorAt(Token* token, const char* message);
  void error(const char* message) { errorAt(&parser.previous, message); }
  void errorAtCurrent(const char* message) {
    errorAt(&parser.current, message);
  }
};

// parse rule table
typedef void (*ParseFn)(Compiler*);

class ParseRule {
 public:
  ParseFn prefix;
  ParseFn infix;
  ParseRule(){};
  ParseRule(ParseFn pre, ParseFn in, Precedence precedence)
      : prefix(pre), infix(in), precedence(precedence){};
  Precedence precedence;
};

ParseRule parseRules[TokenType::TOKEN_TYPE_NUMS];
ParseRule* getRule(TokenType type) { return &parseRules[type]; };

void number(Compiler* compiler);
void grouping(Compiler* compiler);
void unary(Compiler* compiler);
void binary(Compiler* compiler);

void initializeParseRules() {
  parseRules[TOKEN_LEFT_PAREN] = ParseRule{grouping, NULL, PREC_NONE};
  parseRules[TOKEN_RIGHT_PAREN] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_LEFT_BRACE] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_RIGHT_BRACE] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_COMMA] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_DOT] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_MINUS] = ParseRule{unary, binary, PREC_TERM};
  parseRules[TOKEN_PLUS] = ParseRule{NULL, binary, PREC_TERM};
  parseRules[TOKEN_SEMICOLON] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_SLASH] = ParseRule{NULL, binary, PREC_FACTOR};
  parseRules[TOKEN_STAR] = ParseRule{NULL, binary, PREC_FACTOR};
  parseRules[TOKEN_BANG] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_BANG_EQUAL] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_EQUAL] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_EQUAL_EQUAL] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_GREATER] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_GREATER_EQUAL] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_LESS] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_LESS_EQUAL] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_IDENTIFIER] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_STRING] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_NUMBER] = ParseRule{number, NULL, PREC_NONE};
  parseRules[TOKEN_AND] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_CLASS] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_ELSE] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_FALSE] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_FOR] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_FUN] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_IF] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_NIL] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_OR] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_PRINT] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_RETURN] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_SUPER] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_THIS] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_TRUE] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_VAR] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_WHILE] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_ERROR] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_EOF] = ParseRule{NULL, NULL, PREC_NONE};
}
#endif
