#ifndef ROBLANG_LEXER_H
#define ROBLANG_LEXER_H

typedef enum {
  TOKEN_IDENTIFIER,
  TOKEN_NUMBER,
  TOKEN_BRACKET,
  TOKEN_ARROW,
  TOKEN_OPERATOR,
  TOKEN_DELIMITER
} TokenType;

typedef struct {
  int col;
  int row;
  TokenType type;
  char *value;
} Token;

int lex(char *, Token ***);

#endif
