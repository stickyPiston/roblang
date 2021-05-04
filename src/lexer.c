#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <roblang/lexer.h>

#define addToken(condition, tokenType) \
      char *identifier = NULL; \
      int stringSize = 0; \
      while (condition) { \
        if (stringSize == 0) { \
          identifier = malloc(sizeof(char)); \
          identifier[stringSize++] = script[lexIndex++]; \
        } else { \
          identifier = realloc(identifier, ++stringSize * sizeof(char)); \
          identifier[stringSize - 1] = script[lexIndex++]; \
        } \
        col++; \
      } \
      identifier = realloc(identifier, ++stringSize * sizeof(char)); \
      identifier[stringSize - 1] = '\0'; \
      Token *token = malloc(sizeof(Token)); \
      *token = (Token){ .col = col, .row = row, .value = identifier, .type = tokenType }; \

int lexIndex = 0, col = 1, row = 1;
char *script = NULL;

void setScript(char *set) {
  script = set;
  lexIndex = 0; col = 1; row = 1; 
}

Token *lexNextToken() {
  if (lexIndex >= strlen(script))
    return NULL;

  if (isspace(script[lexIndex])) { // Remove spaces and line breaks
    while (isspace(script[lexIndex])) {
      if (script[lexIndex] == ' ') col++;
      else if (script[lexIndex] == '\n' || script[lexIndex] == '\r') {
        row++;
        col = 1;
      }
      lexIndex++;
    }
  }

  char firstChar = script[lexIndex];
  if (firstChar == '#') {
    lexIndex++; col++;
    addToken(script[lexIndex] != '#', -1);
    lexIndex++; col++;
    return lexNextToken();
  } else if (isalpha(firstChar) || firstChar == '_') { // identifier
    addToken(isalpha(script[lexIndex]) || isdigit(script[lexIndex]) || script[lexIndex] == '_', TOKEN_IDENTIFIER);
    return token;
  } else if (isdigit(firstChar)) { // number
    addToken(isdigit(script[lexIndex]), TOKEN_NUMBER);
    return token;
  } else if (firstChar == '(' || firstChar == ')' || firstChar == '{' || firstChar == '}' || firstChar == '[' || firstChar == ']') { // bracket
    char *identifier = calloc(2, 1);
    identifier[0] = firstChar;
    lexIndex++; col++;
    Token *token = malloc(sizeof(Token));
    *token = (Token){ .col = col, .row = row, .value = identifier, .type = TOKEN_BRACKET };
    return token;
  } else if (firstChar == '-' && script[lexIndex + 1] == '>') { // arrow
    lexIndex += 2; col += 2;
    char *identifier = calloc(3, 1);
    strcpy(identifier, "->");
    Token *token = malloc(sizeof(Token));
    *token = (Token){ .col = col, .row = row, .value = identifier, .type = TOKEN_ARROW };
    return token;
  } else if (
    firstChar == '+' || firstChar == '-' || firstChar == '*' ||
    firstChar == '/' || firstChar == '<' || firstChar == '>' ||
    firstChar == '=' || firstChar == '!' || firstChar == '&' ||
    firstChar == '|' || firstChar == '~'
  ) { // operator
    char *identifier = NULL;
    if (
      (script[lexIndex + 1] == '=' && (firstChar != '+' && firstChar != '-' && firstChar != '*' && firstChar != '/')) ||
      (script[lexIndex + 1] == '>' && firstChar == '>') ||
      (script[lexIndex + 1] == '<' && firstChar == '<') ||
      (script[lexIndex + 1] == '&' && firstChar == '&') ||
      (script[lexIndex + 1] == '|' && firstChar == '|')
    ) {
      identifier = calloc(3, sizeof(char));
      identifier[0] = firstChar;
      identifier[1] = script[lexIndex + 1];
      lexIndex += 2; col += 2;
    } else {
      identifier = calloc(2, sizeof(char));
      identifier[0] = firstChar;
      lexIndex++; col++;
    }
    
    Token *token = malloc(sizeof(Token));
    *token = (Token){ .col = col, .row = row, .value = identifier, .type = TOKEN_OPERATOR };
    return token;
  } else if (firstChar == ';' || firstChar == ',') { // delimiter
    char *identifier = calloc(2, 1);
    identifier[0] = firstChar;
    lexIndex++; col++;
    Token *token = malloc(sizeof(Token));
    *token = (Token){ .col = col, .row = row, .value = identifier, .type = TOKEN_DELIMITER };
    return token;
  } else if (firstChar == '"') {
    lexIndex++;
    addToken(script[lexIndex] != '"', TOKEN_STRINGLITERAL);
    lexIndex++;
    return token;
  } else {
    // Unrecognised token
    return NULL;
  }

  return NULL;
}

Token *peekNextToken() {
  int savedIndex = lexIndex, savedCol = col, savedRow = row;
  Token *token = lexNextToken();
  lexIndex = savedIndex; col = savedCol; row = savedRow;
  return token;
}
