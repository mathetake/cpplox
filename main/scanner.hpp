#ifndef clox_scanner_h
#define clox_scanner_h

enum TokenType {
  // Single-character tokens.
  TOKEN_LEFT_PAREN,
  TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE,
  TOKEN_RIGHT_BRACE,
  TOKEN_COMMA,
  TOKEN_DOT,
  TOKEN_MINUS,
  TOKEN_PLUS,
  TOKEN_SEMICOLON,
  TOKEN_SLASH,
  TOKEN_STAR,

  // One or two character tokens.
  TOKEN_BANG,
  TOKEN_BANG_EQUAL,
  TOKEN_EQUAL,
  TOKEN_EQUAL_EQUAL,
  TOKEN_GREATER,
  TOKEN_GREATER_EQUAL,
  TOKEN_LESS,
  TOKEN_LESS_EQUAL,

  // Literals.
  TOKEN_IDENTIFIER,
  TOKEN_STRING,
  TOKEN_NUMBER,

  // Keywords.
  TOKEN_AND,
  TOKEN_CLASS,
  TOKEN_ELSE,
  TOKEN_FALSE,
  TOKEN_FOR,
  TOKEN_FUN,
  TOKEN_IF,
  TOKEN_NIL,
  TOKEN_OR,
  TOKEN_PRINT,
  TOKEN_RETURN,
  TOKEN_SUPER,
  TOKEN_THIS,
  TOKEN_TRUE,
  TOKEN_VAR,
  TOKEN_WHILE,

  TOKEN_ERROR,
  TOKEN_EOF,

  TOKEN_TYPE_NUMS = TOKEN_EOF + 1,
};

class Token {
 public:
  TokenType type;
  const char* start;
  int length;
  int line;
  Token(){};
  Token(TokenType type);
  ~Token(){};
};

class Scanner {
 public:
  const char* start;
  const char* current;
  int line;
  Scanner(const char* source) : start(source), current(source), line(1){};
  Scanner(){};
  ~Scanner(){};

  bool isAtEnd() { return *current == '\0'; }
  bool match(char expected);
  char advance();
  void skipWhitespace();
  char peek();
  char peekNext();
  Token stringLiteral();
  Token number();
  Token identifier();
  TokenType identifierType();
  TokenType checkKeyword(int begin, int length, const char* rest,
                         TokenType type);
  Token scanToken();
  Token makeToken(TokenType type);
  Token errorToken(const char* message);
};

#endif
