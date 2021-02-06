#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <roblang/lexer.h>

#define addToArray(array, arraySize, item) \
  if (arraySize == 0) { \
    array = malloc(sizeof(item));\
    array[0] = item; \
    arraySize++; \
  } else { \
    array = realloc(array, ++arraySize * sizeof(item));\
    array[arraySize - 1] = item; \
  } \

#define addToken(condition, tokenType) \
      char *identifier = NULL; \
      int stringSize = 0; \
      while (condition) { \
        addToArray(identifier, stringSize, script[index++]); \
        col++; \
      } \
      identifier = realloc(identifier, ++stringSize * sizeof(char)); \
      identifier[stringSize - 1] = '\0'; \
      Token *token = malloc(sizeof(Token)); \
      *token = (Token){ .col = col, .row = row, .value = identifier, .type = tokenType }; \

int lex(char *script, Token ***array) {
  int size = 0, index = 0, col = 1, row = 1, scriptSize = strlen(script);

  while (index < scriptSize) {
    char firstChar = script[index];
    if (isspace(script[index])) { // Remove spaces and line breaks
      while (isspace(script[index])) {
        if (script[index] == ' ') col++;
        else if (script[index] == '\n' || script[index] == '\r') {
          row++;
          col = 1;
        }
        index++;
      }
      continue;
    } else if (firstChar == '#') {
      index++; col++;
      addToken(script[index] != '#', -1);
      index++; col++;
    } else if (isalpha(firstChar)) { // identifier
      addToken(isalpha(script[index]) || isdigit(script[index]) || script[index] == '_', TOKEN_IDENTIFIER);
      addToArray(*array, size, token);
    } else if (isdigit(firstChar)) { // number
      addToken(isdigit(script[index]), TOKEN_NUMBER);
      addToArray(*array, size, token);
    } else if (firstChar == '(' || firstChar == ')' || firstChar == '{' || firstChar == '}' || firstChar == '[' || firstChar == ']') { // bracket
      char identifier[2] = { firstChar, '\0' };
      index++; col++;
      Token *token = malloc(sizeof(Token));
      *token = (Token){ .col = col, .row = row, .value = identifier, .type = TOKEN_BRACKET };
      addToArray(*array, size, token);
    } else if (firstChar == '-' && script[index + 1] == '>') { // arrow
      index += 2; col += 2;
      char *identifier = "->";
      Token *token = malloc(sizeof(Token));
      *token = (Token){ .col = col, .row = row, .value = identifier, .type = TOKEN_ARROW };
      addToArray(*array, size, token);
    } else if (
      firstChar == '+' || firstChar == '-' || firstChar == '*' ||
      firstChar == '/' || firstChar == '<' || firstChar == '>' ||
      firstChar == '=' || firstChar == '!' || firstChar == '&' ||
      firstChar == '|'
    ) { // operator
      char *identifier = NULL;
      if (
        (script[index + 1] == '=' && (firstChar != '+' && firstChar != '-' && firstChar != '*' && firstChar != '/')) ||
        (script[index + 1] == '>' && firstChar == '>') ||
        (script[index + 1] == '<' && firstChar == '<') ||
        (script[index + 1] == '&' && firstChar == '&') ||
        (script[index + 1] == '|' && firstChar == '|')
      ) {
        identifier = calloc(3, sizeof(char));
        identifier[0] = firstChar;
        identifier[1] = script[index + 1];
        index += 2; col += 2;
      } else {
        identifier = calloc(2, sizeof(char));
        identifier[0] = firstChar;
        index++; col++;
      }
      
      Token *token = malloc(sizeof(Token));
      *token = (Token){ .col = col, .row = row, .value = identifier, .type = TOKEN_OPERATOR };
      addToArray(*array, size, token);
    } else if (firstChar == ';' || firstChar == ',') { // delimiter
      char identifier[2] = { firstChar, '\0' };
      index++; col++;
      Token *token = malloc(sizeof(Token));
      *token = (Token){ .col = col, .row = row, .value = identifier, .type = TOKEN_DELIMITER };
      addToArray(*array, size, token);
    }
   
  }

  return size;
}
