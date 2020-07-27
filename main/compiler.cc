
#include "compiler.hpp"

Compiler::Compiler(const char* source, Chunk* targetChunk) {
  scanner = Scanner(source);
  parser = Parser{};
  chunk = targetChunk;

  initializeParseRules();
}

bool Compiler::compile() {
  advance();
  expression();
  consume(TokenType::TOKEN_EOF, "Expect end of expression.");
  endCompiler();
  return !parser.hadError;
}

void Compiler::emitReturn() { emitByte(OptCode::OP_RETURN); };

void Compiler::endCompiler() { emitReturn(); }

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
  compiler->emitConstant(value);
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
    default:
      return;
  }
}

void binary(Compiler* compiler) {
  TokenType type = compiler->parser.previous.type;

  auto rule = getRule(type);
  compiler->parsePrecedence(static_cast<Precedence>(rule->precedence + 1));

  switch (type) {
    case TOKEN_PLUS:
      compiler->emitByte(OP_ADD);
      break;
    case TOKEN_MINUS:
      compiler->emitByte(OP_SUBTRACT);
      break;
    case TOKEN_STAR:
      compiler->emitByte(OP_MULTIPLY);
      break;
    case TOKEN_SLASH:
      compiler->emitByte(OP_DIVIDE);
      break;
    default:
      return;
  }
}
