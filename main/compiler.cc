
#include "compiler.hpp"

#ifdef DEBUG_PRINT_CODE
#include "debug.hpp"
#endif

ParseRule parseRules[TokenType::TOKEN_TYPE_NUMS];
ParseRule* getRule(TokenType type) { return &parseRules[type]; };

void initializeParseRules() {
  parseRules[TOKEN_LEFT_PAREN] = ParseRule{grouping, call, PREC_CALL};
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
  parseRules[TOKEN_AND] = ParseRule{NULL, andOp, PREC_AND};
  parseRules[TOKEN_CLASS] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_ELSE] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_FALSE] = ParseRule{literal, NULL, PREC_NONE};
  parseRules[TOKEN_FOR] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_FUN] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_IF] = ParseRule{NULL, NULL, PREC_NONE};
  parseRules[TOKEN_NIL] = ParseRule{literal, NULL, PREC_NONE};
  parseRules[TOKEN_OR] = ParseRule{NULL, orOp, PREC_OR};
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

Compiler::Compiler(const char* source, FunctionType functionType,
                   Table* stringTable, Obj** objects)
    : scanner(new Scanner(source)),
      objects(objects),
      stringTable(stringTable),
      localCount(0),
      scopeDepth(0),
      functionType(functionType),
      parser(new Parser{}),
      enclosing(nullptr) {
  initializeParseRules();

  function = allocateFunctionObject(objects);
  Local* local = &locals[localCount++];
  local->depth = 0;
  local->name.start = "";
  local->name.length = 0;
}

Compiler::Compiler(Compiler* parent, FunctionType functionType)
    : scanner(parent->scanner),
      objects(parent->objects),
      stringTable(parent->stringTable),
      localCount(0),
      scopeDepth(0),
      functionType(functionType),
      parser(parent->parser),
      enclosing(parent) {
  function = allocateFunctionObject(objects);

  if (functionType != TYPE_SCRIPT) {
    function->name =
        new ObjString(parser->previous.start, parser->previous.length);
  }

  Local* local = &locals[localCount++];
  local->depth = 0;
  local->name.start = "";
  local->name.length = 0;
}

ObjFunction* Compiler::compile() {
  advance();
  while (!match(TokenType::TOKEN_EOF)) {
    declaration();
  }

  ObjFunction* function = endCompiler();
  return parser->hadError ? NULL : function;
}

bool Compiler::match(TokenType type) {
  if (!check(type)) return false;
  advance();
  return true;
}

bool Compiler::check(TokenType type) { return parser->current.type == type; }

void Compiler::declaration() {
  if (match(TOKEN_FUN)) {
    functionDeclaration();
  } else if (match(TOKEN_VAR)) {
    varDeclaration();
  } else {
    statement();
  }

  if (parser->panicMode) synchronize();
}

void Compiler::functionDeclaration() {
  uint8_t global = parseVariable("Expect function name.");
  markInitialized();
  compileFunction(TYPE_FUNCTION);
  defineVariable(global);
}

void Compiler::compileFunction(FunctionType type) {
  auto child = Compiler(this, type);
  child.beginScope();
  child.consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");
  if (!child.check(TOKEN_RIGHT_PAREN)) {
    do {
      child.function->arity++;
      if (child.function->arity > 255) {
        child.errorAtCurrent("Cannot have more than 255 parameters.");
      }

      uint8_t paramConstant = child.parseVariable("Expect parameter name.");
      child.defineVariable(paramConstant);
    } while (child.match(TOKEN_COMMA));
  }

  child.consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");

  // The body.
  child.consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
  child.block();

  ObjFunction* function = child.endCompiler();

  emitBytes(OP_CONSTANT, makeConstant(OBJ_VAL(function)));
}

void Compiler::statement() {
  if (match(TOKEN_PRINT)) {
    printStatement();
  } else if (match(TOKEN_IF)) {
    ifStatement();
  } else if (match(TOKEN_WHILE)) {
    whileStatement();
  } else if (match(TOKEN_FOR)) {
    forStatement();
  } else if (match(TOKEN_LEFT_BRACE)) {
    beginScope();
    block();
    endScope();
  } else if (match(TOKEN_RETURN)) {
    returnStatement();
  } else {
    expressionStatement();
  }
}

void Compiler::ifStatement() {
  consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

  int thenJump = emitJump(OP_JUMP_IF_FALSE);
  emitByte(OP_POP);
  statement();

  int elseJump = emitJump(OP_JUMP);

  patchJump(thenJump);
  emitByte(OP_POP);

  if (match(TOKEN_ELSE)) statement();
  patchJump(elseJump);
}

void Compiler::whileStatement() {
  int loopStart = function->chunk.count();

  consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

  int exitJump = emitJump(OP_JUMP_IF_FALSE);

  emitByte(OP_POP);
  statement();

  emitLoop(loopStart);

  patchJump(exitJump);
  emitByte(OP_POP);
}

void Compiler::forStatement() {
  beginScope();

  consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
  if (match(TOKEN_SEMICOLON)) {
  } else if (match(TOKEN_VAR)) {
    varDeclaration();
  } else {
    expressionStatement();
  }

  int loopStart = function->chunk.count();
  int exitJump = -1;
  if (!match(TOKEN_SEMICOLON)) {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

    exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
  }

  if (!match(TOKEN_RIGHT_PAREN)) {
    int bodyJump = emitJump(OP_JUMP);

    int incrementStart = function->chunk.count();
    expression();
    emitByte(OP_POP);
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

    emitLoop(loopStart);
    loopStart = incrementStart;
    patchJump(bodyJump);
  }

  statement();

  emitLoop(loopStart);

  if (exitJump != -1) {
    patchJump(exitJump);
    emitByte(OP_POP);  // Condition.
  }

  endScope();
}

void Compiler::emitLoop(int loopStart) {
  emitByte(OP_LOOP);

  int offset = function->chunk.count() - loopStart + 2;
  if (offset > UINT16_MAX) error("Loop body too large.");

  emitByte((offset >> 8) & 0xff);
  emitByte(offset & 0xff);
}

int Compiler::emitJump(uint8_t instruction) {
  emitByte(instruction);
  emitByte(0xff);
  emitByte(0xff);
  return function->chunk.count() - 2;
}

void Compiler::patchJump(int offset) {
  int jump = function->chunk.count() - offset - 2;
  if (jump > UINT16_MAX) {
    error("Too much code to jump over.");
  }

  function->chunk.code[offset] = (jump >> 8) & 0xff;
  function->chunk.code[offset + 1] = jump & 0xff;
}

void Compiler::block() {
  while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
    declaration();
  }
  consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

void Compiler::beginScope() { scopeDepth++; }

void Compiler::endScope() {
  scopeDepth--;
  while (localCount > 0 && locals[localCount - 1].depth > scopeDepth) {
    emitByte(OP_POP);
    localCount--;
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
  if (scopeDepth > 0) {
    markInitialized();
    return;
  }
  emitBytes(OP_DEFINE_GLOBAL, global);
}

void Compiler::namedVariable(Token name, bool canAssign) {
  uint8_t getOp, setOp;
  auto arg = resolveLocal(&name);
  if (arg != -1) {
    getOp = OP_GET_LOCAL;
    setOp = OP_SET_LOCAL;
  } else {
    arg = identifierConstant(&name);
    getOp = OP_GET_GLOBAL;
    setOp = OP_SET_GLOBAL;
  }

  if (canAssign && match(TokenType::TOKEN_EQUAL)) {
    expression();
    emitBytes(setOp, arg);
  } else {
    emitBytes(getOp, arg);
  }
}

int Compiler::resolveLocal(Token* name) {
  for (int i = localCount - 1; i >= 0; i--) {
    Local* local = &locals[i];
    if (identifiersEqual(&local->name, name)) {
      return i;
    }
  }
  return -1;
}

uint8_t Compiler::parseVariable(const char* errorMessage) {
  consume(TOKEN_IDENTIFIER, errorMessage);

  if (scopeDepth > 0) {
    // local variables
    declareVariable();
    return 0;
  }

  return identifierConstant(&parser->previous);
}

void Compiler::declareVariable() {
  Token* name = &parser->previous;
  for (int i = localCount - 1; i >= 0; i--) {
    Local* local = &locals[i];
    if (local->depth != -1 && local->depth < scopeDepth) {
      break;
    }
    if (identifiersEqual(name, &local->name)) {
      error("Variable with this name already declared in this scope.");
    }
  }

  addLocal(*name);
}

void Compiler::addLocal(Token name) {
  if (localCount == UINT8_COUNT) {
    error("Too many local variables in function.");
    return;
  }
  Local* local = &locals[localCount++];
  local->name = name;
  local->depth = -1;
}

void Compiler::markInitialized() {
  if (scopeDepth == 0) return;
  locals[localCount - 1].depth = scopeDepth;
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

void Compiler::returnStatement() {
  if (functionType == TYPE_SCRIPT) {
    error("Cannot return from top-level code.");
  }

  if (match(TOKEN_SEMICOLON)) {
    emitReturn();
  } else {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
    emitByte(OP_RETURN);
  }
}

void Compiler::emitReturn() {
  emitByte(OP_NIL);
  emitByte(OptCode::OP_RETURN);
};

ObjFunction* Compiler::endCompiler() {
  emitReturn();
  ObjFunction* ret = function;
#ifdef DEBUG_PRINT_CODE
  if (!parser->hadError) {
    disassembleChunk(&function->chunk, function->name != nullptr
                                           ? function->name->str.c_str()
                                           : "script");
  }
#endif
  return ret;
}

void Compiler::advance() {
  parser->previous = parser->current;

  while (true) {
    parser->current = scanner->scanToken();
    if (parser->current.type != TokenType::TOKEN_ERROR) break;

    errorAtCurrent(parser->current.start);
  }
}

void Compiler::consume(TokenType type, const char* message) {
  if (parser->current.type == type) {
    advance();
    return;
  }
  errorAtCurrent(message);
}

void Compiler::errorAt(Token* token, const char* message) {
  if (parser->panicMode) return;
  parser->panicMode = true;

  fprintf(stderr, "[line %d] Error", token->line);

  if (token->type == TokenType::TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token->type == TOKEN_ERROR) {
    // Nothing.
  } else {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);
  parser->hadError = true;
}
void Compiler::synchronize() {
  parser->panicMode = false;

  while (parser->current.type != TOKEN_EOF) {
    if (parser->previous.type == TOKEN_SEMICOLON) return;

    switch (parser->current.type) {
      case TOKEN_CLASS:
      case TOKEN_FUN:
      case TOKEN_VAR:
      case TOKEN_FOR:
      case TOKEN_IF:
      case TOKEN_WHILE:
      case TOKEN_PRINT:
      case TOKEN_RETURN:
        return;
      default:;
    }
    advance();
  }
}

void Compiler::emitByte(uint8_t byte) {
  function->chunk.write_chunk(byte, parser->previous.line);
}

void Compiler::emitConstant(Value value) {
  emitBytes(OptCode::OP_CONSTANT, makeConstant(value));
}

uint8_t Compiler::makeConstant(Value value) {
  int constant = function->chunk.add_const(value);
  if (constant > UINT8_MAX) {
    error("Too many constants in one chunk.");
    return 0;
  }
  return static_cast<uint8_t>(constant);
}

void Compiler::expression() { parsePrecedence(Precedence::PREC_ASSIGNMENT); }

void Compiler::parsePrecedence(Precedence precedence) {
  advance();
  auto prefixRule = getRule(parser->previous.type)->prefix;
  if (prefixRule == nullptr) {
    return;
  }

  bool canAssign = precedence <= PREC_ASSIGNMENT;
  prefixRule(this, canAssign);

  while (precedence <= getRule(parser->current.type)->precedence) {
    advance();
    auto infixRule = getRule(parser->previous.type)->infix;
    infixRule(this, canAssign);
  }
};

void number(Compiler* compiler, bool canAssign) {
  double value = strtod(compiler->parser->previous.start, NULL);
  compiler->emitConstant(NUMBER_VAL(value));
}

void grouping(Compiler* compiler, bool canAssign) {
  compiler->expression();
  compiler->consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

void unary(Compiler* compiler, bool canAssign) {
  auto type = compiler->parser->previous.type;

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

void binary(Compiler* compiler, bool canAssign) {
  TokenType type = compiler->parser->previous.type;

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

void literal(Compiler* compiler, bool canAssign) {
  switch (compiler->parser->previous.type) {
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

void string(Compiler* compiler, bool canAssign) {
  compiler->emitConstant(
      OBJ_VAL(allocateStringObject(compiler->parser->previous.start + 1,
                                   compiler->parser->previous.length - 2,
                                   compiler->stringTable, compiler->objects)));
}

void variable(Compiler* compiler, bool canAssign) {
  compiler->namedVariable(compiler->parser->previous, canAssign);
}

void andOp(Compiler* compiler, bool canAssign) {
  int endJump = compiler->emitJump(OP_JUMP_IF_FALSE);

  compiler->emitByte(OP_POP);
  compiler->parsePrecedence(PREC_AND);

  compiler->patchJump(endJump);
}

void orOp(Compiler* compiler, bool canAssign) {
  int elseJump = compiler->emitJump(OP_JUMP_IF_FALSE);
  int endJump = compiler->emitJump(OP_JUMP);

  compiler->patchJump(elseJump);
  compiler->emitByte(OP_POP);

  compiler->parsePrecedence(PREC_OR);
  compiler->patchJump(endJump);
}

void call(Compiler* compiler, bool canAssign) {
  uint8_t argCount = compiler->argumentList();
  compiler->emitBytes(OP_CALL, argCount);
}

uint8_t Compiler::argumentList() {
  uint8_t argCount = 0;
  if (!check(TOKEN_RIGHT_PAREN)) {
    do {
      expression();
      if (argCount == 255) {
        error("Cannot have more than 255 arguments.");
      }
      argCount++;
    } while (match(TOKEN_COMMA));
  }

  consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
  return argCount;
}
