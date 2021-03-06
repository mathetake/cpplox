#ifndef cpplox_compiler_h
#define cpplox_compiler_h
#include "chunk.hpp"
#include "common.hpp"
#include "object.hpp"
#include "scanner.hpp"
#include "table.hpp"

enum FunctionType {
  TYPE_FUNCTION,
  TYPE_SCRIPT,
};

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

class Local {
 public:
  Token name;
  int depth;
  bool isCaptured;
};

class Upvalue {
 public:
  uint8_t index;
  bool isLocal;
};

class Compiler {
 public:
  Scanner* scanner;
  Parser* parser;
  Table* stringTable;
  Obj** objects;

  FunctionType functionType;
  ObjFunction* function;

  Upvalue upvalues[UINT8_COUNT];
  Local locals[UINT8_COUNT];
  int localCount;
  int scopeDepth;

  Compiler(const char* source, FunctionType functionType, Table* stringTable,
           Obj** objects);

  Compiler(const char* source, Table* stringTable, Obj** objects)
      : Compiler(source, FunctionType::TYPE_SCRIPT, stringTable, objects){};

  Compiler* enclosing;
  Compiler(Compiler* parent, FunctionType type);
  void freeCompiler() { delete parser, delete scanner; };

  ObjFunction* compile();
  void advance();
  void consume(TokenType type, const char* message);
  void emitByte(uint8_t byte);
  void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1), emitByte(byte2);
  };
  ObjFunction* endCompiler();
  void emitReturn();
  void emitConstant(Value value);
  uint8_t makeConstant(Value value);

  bool match(TokenType type);
  bool check(TokenType type);

  void statement();
  void declaration();
  void ifStatement();
  void whileStatement();
  void forStatement();
  void returnStatement();
  void block();
  void printStatement();
  void expressionStatement();
  void varDeclaration();
  void functionDeclaration();
  void declareVariable();
  void compileFunction(FunctionType type);
  uint8_t argumentList();
  uint8_t parseVariable(const char* errorMessage);
  uint8_t identifierConstant(const Token* name);
  void defineVariable(uint8_t global);
  void namedVariable(Token name, bool canAssign);

  void parsePrecedence(Precedence precedence);
  void expression();

  void beginScope();
  void endScope();
  void addLocal(Token name);
  void markInitialized();
  int resolveLocal(Token* name);
  int resolveUpvalue(Token* name);
  int addUpvalue(uint8_t index, bool isLocal);

  int emitJump(uint8_t instruction);
  void patchJump(int offset);
  void emitLoop(int loopStart);

  // errors
  void errorAt(Token* token, const char* message);
  void error(const char* message) { errorAt(&parser->previous, message); }
  void errorAtCurrent(const char* message) {
    errorAt(&parser->current, message);
  }
  void synchronize();
};

void number(Compiler* compiler, bool canAssign);
void grouping(Compiler* compiler, bool canAssign);
void unary(Compiler* compiler, bool canAssign);
void binary(Compiler* compiler, bool canAssign);
void literal(Compiler* compiler, bool canAssign);
void string(Compiler* compiler, bool canAssign);
void variable(Compiler* compiler, bool canAssign);
void andOp(Compiler* compiler, bool canAssign);
void orOp(Compiler* compiler, bool canAssign);
void call(Compiler* compiler, bool canAssign);

// parse rule table
using ParseFn = void(Compiler*, bool);
using ParseFnPtr = ParseFn*;

class ParseRule {
 public:
  ParseFnPtr prefix;
  ParseFnPtr infix;
  ParseRule(){};
  ParseRule(ParseFnPtr pre, ParseFnPtr in, Precedence precedence)
      : prefix(pre), infix(in), precedence(precedence){};
  Precedence precedence;
};

void initializeParseRules();

#endif
