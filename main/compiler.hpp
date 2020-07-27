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
void initializeParseRules();

void number(Compiler* compiler);
void grouping(Compiler* compiler);
void unary(Compiler* compiler);
void binary(Compiler* compiler);
#endif
