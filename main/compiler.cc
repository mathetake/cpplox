
#include "compiler.hpp"

#ifdef DEBUG_PRINT_CODE
#include "debug.hpp"
#endif

ParseRule parseRules[TokenType::TOKEN_TYPE_NUMS];
ParseRule* getRule(TokenType type) { return &parseRules[type]; };

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
  parseRules[TOKEN_BANG] = ParseRule{unary, NULL, PREC_NONE};
  parseRules[TOKEN_BANG_EQUAL] = ParseRule{NULL, binary, PREC_EQUALITY};
  parseRules[TOKEN_EQUAL] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_EQUAL_EQUAL] = ParseRule{NULL, binary, PREC_EQUALITY};
  parseRules[TOKEN_GREATER] = ParseRule{NULL, binary, PREC_COMPARISON};
  parseRules[TOKEN_GREATER_EQUAL] = ParseRule{NULL, binary, PREC_COMPARISON};
  parseRules[TOKEN_LESS] = ParseRule{NULL, binary, PREC_COMPARISON};
  parseRules[TOKEN_LESS_EQUAL] = ParseRule{NULL, binary, PREC_COMPARISON};
  parseRules[TOKEN_IDENTIFIER] = ParseRule{variable, NULL, PREC_NONE};
  parseRules[TOKEN_STRING] = ParseRule{string, NULL, PREC_NONE};
  parseRules[TOKEN_NUMBER] = ParseRule{number, NULL, PREC_NONE};
  parseRules[TOKEN_AND] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_CLASS] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_ELSE] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_FALSE] = ParseRule{literal, NULL, PREC_NONE};
  parseRules[TOKEN_FOR] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_FUN] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_IF] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_NIL] = ParseRule{literal, NULL, PREC_NONE};
  parseRules[TOKEN_OR] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_PRINT] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_RETURN] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_SUPER] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_THIS] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_TRUE] = ParseRule{literal, NULL, PREC_NONE};
  parseRules[TOKEN_VAR] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_WHILE] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_ERROR] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_EOF] = ParseRule{NULL, NULL, PREC_NONE};
}

Compiler::Compiler(const char* source, Chunk* targetChunk, Table* strTable,
                   Obj** obs) {
  scanner = Scanner(source);
  parser = Parser{};
  chunk = targetChunk;
  objects = obs;
  stringTable = strTable;

  initializeParseRules();
}

bool Compiler::compile() {
  advance();
  while (!match(TokenType::TOKEN_EOF)) {
    declaration();
  }

  endCompiler();
  return !parser.hadError;
}

bool Compiler::match(TokenType type) {
  if (!check(type)) return false;
  advance();
  return true;
}

bool Compiler::check(TokenType type) { return parser.current.type == type; }

void Compiler::declaration() {
  if (match(TOKEN_VAR)) {
    varDeclaration();
  } else {
    statement();
  }

  if (parser.panicMode) synchronize();
}

void Compiler::statement() {
  if (match(TOKEN_PRINT)) {
    printStatement();
  } else {
    expressionStatement();
  }
}

void Compiler::varDeclaration() {
  uint8_t global = parseVariable("Expect variable name.");

  if (match(TOKEN_EQUAL)) {
    expression();
  } else {
    emitByte(OP_NIL);
  }
  consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

  defineVariable(global);
}

void Compiler::defineVariable(uint8_t global) {
  emitBytes(OP_DEFINE_GLOBAL, global);
}
void Compiler::namedVariable(Token name) {
  uint8_t arg = identifierConstant(&name);
  emitBytes(OP_GET_GLOBAL, arg);
}

uint8_t Compiler::parseVariable(const char* errorMessage) {
  consume(TOKEN_IDENTIFIER, errorMessage);
  return identifierConstant(&parser.previous);
}

uint8_t Compiler::identifierConstant(const Token* name) {
  return makeConstant(OBJ_VAL(
      allocateStringObject(name->start, name->length, stringTable, objects)));
}

void Compiler::printStatement() {
  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after value.");
  emitByte(OP_PRINT);
}

void Compiler::expressionStatement() {
  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
  emitByte(OP_POP);
}

void Compiler::emitReturn() { emitByte(OptCode::OP_RETURN); };

void Compiler::endCompiler() {
  emitReturn();
#ifdef DEBUG_PRINT_CODE
  if (!parser.hadError) {
    disassembleChunk(chunk, "code");
  }
#endif
}

void Compiler::advance() {
  parser.previous = parser.current;

  while (true) {
    parser.current = scanner.scanToken();
    if (parser.current.type != TokenType::TOKEN_ERROR) break;

    errorAtCurrent(parser.current.start);
  }
}

void Compiler::consume(TokenType type, const char* message) {
  if (parser.current.type == type) {
    advance();
    return;
  }
  errorAtCurrent(message);
}

void Compiler::errorAt(Token* token, const char* message) {
  if (parser.panicMode) return;
  parser.panicMode = true;

  fprintf(stderr, "[line %d] Error", token->line);

  if (token->type == TokenType::TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token->type == TOKEN_ERROR) {
    // Nothing.
  } else {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);
  parser.hadError = true;
}
void Compiler::synchronize() {
  parser.panicMode = false;

  while (parser.current.type != TOKEN_EOF) {
    if (parser.previous.type == TOKEN_SEMICOLON) return;

    switch (parser.current.type) {
      case TOKEN_CLASS:
      case TOKEN_FUN:
      case TOKEN_VAR:
      case TOKEN_FOR:
      case TOKEN_IF:
      case TOKEN_WHILE:
      case TOKEN_PRINT:
      case TOKEN_RETURN:
        return;

      default:
          // Do nothing.
          ;
    }

    advance();
  }
}

void Compiler::emitByte(uint8_t byte) {
  chunk->write_chunk(byte, parser.previous.line);
}

void Compiler::emitConstant(Value value) {
  emitBytes(OptCode::OP_CONSTANT, makeConstant(value));
}

uint8_t Compiler::makeConstant(Value value) {
  int constant = chunk->add_const(value);
  if (constant > UINT8_MAX) {
    error("Too many constants in one chunk.");
    return 0;
  }
  return static_cast<uint8_t>(constant);
}

void Compiler::expression() { parsePrecedence(Precedence::PREC_ASSIGNMENT); }

void Compiler::parsePrecedence(Precedence precedence) {
  advance();
  auto prefixRule = getRule(parser.previous.type)->prefix;
  if (prefixRule == nullptr) {
    return;
  }
  prefixRule(this);

  while (precedence <= getRule(parser.current.type)->precedence) {
    advance();
    auto infixRule = getRule(parser.previous.type)->infix;
    infixRule(this);
  }
};

void number(Compiler* compiler) {
  double value = strtod(compiler->parser.previous.start, NULL);
  compiler->emitConstant(NUMBER_VAL(value));
}

void grouping(Compiler* compiler) {
  compiler->expression();
  compiler->consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

void unary(Compiler* compiler) {
  auto type = compiler->parser.previous.type;

  compiler->parsePrecedence(Precedence::PREC_UNARY);

  switch (type) {
    case TokenType::TOKEN_MINUS:
      compiler->emitByte(OptCode::OP_NEGATE);
      break;
    case TokenType::TOKEN_BANG:
      compiler->emitByte(OptCode::OP_NOT);
    default:
      return;
  }
}

void binary(Compiler* compiler) {
  TokenType type = compiler->parser.previous.type;

  auto rule = getRule(type);
  compiler->parsePrecedence(static_cast<Precedence>(rule->precedence + 1));

  switch (type) {
    case TokenType::TOKEN_BANG_EQUAL:
      compiler->emitBytes(OP_EQUAL, OP_NOT);
      break;
    case TokenType::TOKEN_EQUAL_EQUAL:
      compiler->emitByte(OP_EQUAL);
      break;
    case TokenType::TOKEN_GREATER:
      compiler->emitByte(OP_GREATER);
      break;
    case TokenType::TOKEN_GREATER_EQUAL:
      compiler->emitBytes(OP_LESS, OP_NOT);
      break;
    case TokenType::TOKEN_LESS:
      compiler->emitByte(OP_LESS);
      break;
    case TokenType::TOKEN_LESS_EQUAL:
      compiler->emitBytes(OP_GREATER, OP_NOT);
      break;
    case TokenType::TOKEN_PLUS:
      compiler->emitByte(OP_ADD);
      break;
    case TokenType::TOKEN_MINUS:
      compiler->emitByte(OP_SUBTRACT);
      break;
    case TokenType::TOKEN_STAR:
      compiler->emitByte(OP_MULTIPLY);
      break;
    case TokenType::TOKEN_SLASH:
      compiler->emitByte(OP_DIVIDE);
      break;
    default:
      return;
  }
}

void literal(Compiler* compiler) {
  switch (compiler->parser.previous.type) {
    case TOKEN_TRUE:
      compiler->emitByte(OP_TRUE);
      break;
    case TOKEN_FALSE:
      compiler->emitByte(OP_FALSE);
      break;
    case TOKEN_NIL:
      compiler->emitByte(OP_NIL);
      break;
    default:
      break;
  }
}

void string(Compiler* compiler) {
  compiler->emitConstant(OBJ_VAL(allocateStringObject(
      compiler->parser.previous.start + 1, compiler->parser.previous.length - 2,
      compiler->stringTable, compiler->objects)));
}

void variable(Compiler* compiler) {
  compiler->namedVariable(compiler->parser.previous);
}