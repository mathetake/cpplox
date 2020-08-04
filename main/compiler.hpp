#ifndef cpplox_compiler_h
#define cpplox_compiler_h
#include "chunk.hpp"
#include "common.hpp"
#include "object.hpp"
#include "scanner.hpp"
#include "table.hpp"

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
  Table* stringTable;
  Obj** objects;

  Compiler(const char* source, Chunk* targetChunk, Table* strTable, Obj** obs);

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

  bool match(TokenType type);
  bool check(TokenType type);

  void statement();
  void declaration();
  void printStatement();
  void expressionStatement();
  void varDeclaration();
  uint8_t parseVariable(const char* errorMessage);
  uint8_t identifierConstant(const Token* name);
  void defineVariable(uint8_t global);
  void namedVariable(Token name);

  void parsePrecedence(Precedence precedence);
  void expression();

  // errors
  void errorAt(Token* token, const char* message);
  void error(const char* message) { errorAt(&parser.previous, message); }
  void errorAtCurrent(const char* message) {
    errorAt(&parser.current, message);
  }
  void synchronize();
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
void literal(Compiler* compiler);
void string(Compiler* compiler);
void variable(Compiler* compiler);
#endif
