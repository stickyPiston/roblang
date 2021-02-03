#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <roblang/lexer.h>

int lex(char *script, Token ***array) {
  int size = 0, index = 0, col = 1, row = 1, scriptSize = strlen(script);

  // test code (will be removed)
  *array = malloc(sizeof(Token *));
  Token *token = malloc(sizeof(Token));
  *token = (Token){ .type = TOKEN_IDENTIFIER, .value = "hello" };
  *array[0] = token;

  return size;
}
