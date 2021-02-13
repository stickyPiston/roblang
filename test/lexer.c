#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#include <roblang/lexer.h>
#include "main.h"

#define stopAtFailure(success, expected, received) \
  if (!success) { \
    printf("Failed test at %s, received %s\n", expected, received); \
    abort(); \
  }

int testTokenIdentifier(void) {
  srand(time(NULL));

  uint8_t success = 1;
  int tries = 100;
  while (tries > 0) {
    int length = (rand() % 30) + 1;
    char *identifier = NULL;
    generateRandomIdentifier(identifier, length);
    setScript(identifier);
    Token *token = lexNextToken();
    success = success && (token != NULL && token->type == TOKEN_IDENTIFIER && strcmp(token->value, identifier) == 0);
    stopAtFailure(success, identifier, token->value)
    tries--;
    free(token);
    free(identifier);
  }

  return success; 
}

int testTokenNumber(void) {
  srand(time(NULL));
  
  uint8_t success = 1;
  int tries = 100;
  while (tries > 0) {
    generateRandomNumber(buffer);
    setScript(buffer);
    Token *token = lexNextToken();
    success = success && (token != NULL && token->type == TOKEN_NUMBER && strcmp(token->value, buffer) == 0);
    stopAtFailure(success, buffer, token->value)
    tries--;
    free(token);
  }
  return success;
}

int testTokenBracket(void) {
  char *brackets[] = { "(", ")", "{", "}", "[", "]" };
  uint8_t success = 1;
  int size = sizeof(brackets) / sizeof(brackets[0]);
  for (int i = 0; i < size; i++) {
    setScript(brackets[i]);
    Token *token = lexNextToken();
    success = success && (token->type == TOKEN_BRACKET && strcmp(token->value, brackets[i]) == 0);
    stopAtFailure(success, brackets[i], token->value);
    free(token);
  }

  return success;
}

int testTokenArrow(void) {
  setScript("->");
  Token *token = lexNextToken();
  int result = (token->type == TOKEN_ARROW && strcmp(token->value, "->") == 0);
  stopAtFailure(result, "->", token->value)
  free(token);
  return result;
}

int testTokenOperator(void) {
  char *operators[] = { "+", "-", "*", "/", "<", ">", "=", "<=", ">=", "!=", "==", "&&", "||", ">>", "<<", "!", "&", "|" };
  uint8_t success = 1;
  int size = sizeof(operators) / sizeof(operators[0]);
  for (int i = 0; i < size; i++) {
    setScript(operators[i]);
    Token *token = lexNextToken();
    success = success && (token->type == TOKEN_OPERATOR && strcmp(token->value, operators[i]) == 0);
    stopAtFailure(success, operators[i], token->value)
    free(token);
  }

  return success;
}

int testTokenDelimiter(void) {
  char *delimiters[] = { ";", "," };
  uint8_t success = 1;
  int size = sizeof(delimiters) / sizeof(delimiters[0]);
  for (int i = 0; i < size; i++) {
    setScript(delimiters[i]);
    Token *token = lexNextToken();
    success = success && (token->type == TOKEN_DELIMITER && strcmp(token->value, delimiters[i]) == 0);
    stopAtFailure(success, delimiters[i], token->value);
    free(token);
  }

  return success;
}

int testTokenComments(void) {
  srand(time(NULL));

  uint8_t success = 1;
  int tries = 100;
  while (tries > 0) {
    int length = (rand() % 30) + 2;
    char *string = calloc(length + 1, sizeof(char));
    string[0] = '#';
    for (int i = 1; i < length - 1; i++) {
      char character;
      do {
        character = rand() % ('~' - ' ') + ' ';
        string[i] = character;
      } while (character == '#');
    }
    string[length - 1] = '#';
    setScript(string);
    Token *token = lexNextToken();
    success = success && (token == NULL);
    if (!success) abort();
    tries--;
    free(string);
    free(token);
  }

  return success; 
}

int testTokenStringLiteral(void) {
  srand(time(NULL));

  uint8_t success = 1;
  int tries = 100;
  while (tries > 0) {
    int length = (rand() % 30) + 2;
    char *string = calloc(length + 1, sizeof(char));
    string[0] = '"';
    for (int i = 1; i < length - 1; i++) {
      char character;
      do {
        character = rand() % ('~' - ' ') + ' ';
        string[i] = character;
      } while (character == '"' || character == '\\');
    }
    string[length - 1] = '"';
    setScript(string);
    Token *token = lexNextToken();
    char *filteredString = calloc(length, sizeof(char));
    strcpy(filteredString, string + 1);
    filteredString[length - 2] = '\0';
    success = success && (token->type == TOKEN_STRINGLITERAL && strcmp(token->value, filteredString) == 0);
    stopAtFailure(success, token->value, filteredString);
    tries--;
    free(token);
    free(string); free(filteredString);
  }

  return success; 
}

TestCase tests[] = {
  { testTokenIdentifier, "identifier tokens are recognised" },
  { testTokenNumber, "number tokens are recognised" },
  { testTokenArrow, "arrow token is recognised" },
  { testTokenBracket, "bracket tokens are recognised" },
  { testTokenOperator, "operator tokens are recognised" },
  { testTokenDelimiter, "delimiter tokens are recognised" },
  { testTokenComments, "comments are ignored" },
  { testTokenStringLiteral, "string literals are recognised" }
};

int main(void) {
  runTests(tests);
}
