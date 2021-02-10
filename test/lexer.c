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
    break; \
  }

int testTokenIdentifier(void) {
  srand(time(NULL));

  uint8_t success = 1;
  int tries = 100;
  while (tries > 0) {
    int length = (rand() % 30) + 1;
    char *identifier = calloc(length + 1, sizeof(char));
    char character = (rand() % 26) + 'A';
    identifier[0] = character;
    for (int i = 1; i < length; i++) {
      uint8_t type = rand() % 4;
      char character;
      switch (type) {
        case 0: character = rand() % 10 + '0'; break;
        case 1: character = rand() % 26 + 'A'; break;
        case 2: character = rand() % 26 + 'a'; break;
        case 3: character = '_'; break;
      }
      identifier[i] = character;
    }
    Token **tokens = NULL;
    lex(identifier, &tokens);
    success = success && (tokens != NULL && tokens[0]->type == TOKEN_IDENTIFIER && strcmp(tokens[0]->value, identifier) == 0);
    stopAtFailure(success, identifier, tokens[0]->value)
    tries--;
    free(tokens);
    free(identifier);
  }

  return success; 
}

int testTokenNumber(void) {
  srand(time(NULL));
  
  uint8_t success = 1;
  int tries = 100;
  while (tries > 0) {
    uint32_t randomNum = rand() % 4294967296;
    char buffer[12];
    snprintf(buffer, 11, "%d", randomNum);
    Token **tokens = NULL;
    lex(buffer, &tokens);
    success = success && (tokens != NULL && tokens[0]->type == TOKEN_NUMBER && strcmp(tokens[0]->value, buffer) == 0);
    tries--;
    free(tokens);
  }
  return success;
}

int testTokenBracket(void) {
  char *brackets[] = { "(", ")", "{", "}", "[", "]" };
  uint8_t success = 1;
  int size = sizeof(brackets) / sizeof(brackets[0]);
  for (int i = 0; i < size; i++) {
    Token **tokens = NULL;
    lex(brackets[i], &tokens);
    success = success && (tokens[0]->type == TOKEN_BRACKET && strcmp(tokens[0]->value, brackets[i]) == 0);
    free(tokens);
  }

  return success;
}

int testTokenArrow(void) {
  Token **tokens = NULL;
  lex("->", &tokens);
  int result = (tokens[0]->type == TOKEN_ARROW && strcmp(tokens[0]->value, "->") == 0);
  free(tokens);
  return result;
}

int testTokenOperator(void) {
  char *operators[] = { "+", "-", "*", "/", "<", ">", "=", "<=", ">=", "!=", "==", "&&", "||", ">>", "<<", "!", "&", "|" };
  uint8_t success = 1;
  int size = sizeof(operators) / sizeof(operators[0]);
  for (int i = 0; i < size; i++) {
    Token **tokens = NULL;
    lex(operators[i], &tokens);
    success = success && (tokens[0]->type == TOKEN_OPERATOR && strcmp(tokens[0]->value, operators[i]) == 0);
    free(tokens);
  }

  return success;
}

int testTokenDelimiter(void) {
  char *delimiters[] = { ";", "," };
  uint8_t success = 1;
  int size = sizeof(delimiters) / sizeof(delimiters[0]);
  for (int i = 0; i < size; i++) {
    Token **tokens = NULL;
    lex(delimiters[i], &tokens);
    success = success && (tokens[0]->type == TOKEN_DELIMITER && strcmp(tokens[0]->value, delimiters[i]) == 0);
    free(tokens);
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
    Token **tokens = NULL;
    lex(string, &tokens);
    success = success && (tokens == NULL);
    stopAtFailure(success, string, "NULL");
    tries--;
    free(tokens);
    free(string);
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
    Token **tokens = NULL;
    lex(string, &tokens);
    char *filteredString = calloc(length, sizeof(char));
    strcpy(filteredString, string + 1);
    filteredString[length - 2] = '\0';
    success = success && (tokens[0]->type == TOKEN_STRINGLITERAL && strcmp(tokens[0]->value, filteredString) == 0);
    stopAtFailure(success, tokens[0]->value, filteredString);
    tries--;
    free(tokens);
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