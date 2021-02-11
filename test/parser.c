#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <roblang/parser.h>
#include "main.h"

#define abortOnFail(reason) \
  if (!success) { \
    printf("Failed test because of %s\n", reason); \
    abort(); \
  } \

#define lexAndParse(script) \
  Token **tokens = NULL; \
  int tokensLength = lex(script, &tokens); \
  Node **nodes = NULL; \
  int nodesLength = parse(tokens, tokensLength, &nodes);

int testBinopNode(void) {
  uint8_t success = 1;

  for (int tries = 100; tries > 0; tries--) {
    char *binops[] = {"=", "+", "-", "/", "*", "<", ">", "<=", ">=", "||", "&&", "|", "&", "<<", ">>", "==", "!="};
    srand(time(NULL));
    uint8_t isIdentifier = rand() % 2 == 0;
    char *operands[2] = { NULL, NULL };
    for (int i = 0; i < 2; i++) {
      generateRandomIdentifier(operands[i], (rand() % 30) + 1);
    }
    char *randomBinop = binops[rand() % (sizeof(binops) / sizeof(binops[0]))];
    char *script = malloc(strlen(randomBinop) + strlen(operands[0]) + strlen(operands[1]) + 1);
    strcpy(script, operands[0]); strcat(script, randomBinop); strcat(script, operands[1]);
    lexAndParse(script);

    if (
      nodes[0]->type == NODE_BINOP &&
      strcmp(nodes[0]->content.binopNode->LHS->content.identifierNode->name, operands[0]) == 0 &&
      strcmp(nodes[0]->content.binopNode->RHS->content.identifierNode->name, operands[1]) == 0
    ) {
      success = 1;
    } else {
      success = 0;
      break;
    }
    free(nodes[0]); free(nodes);
    freeArray(tokens, tokensLength); free(tokens);
    free(operands[0]); free(operands[1]);
    free(script); free(randomBinop);
  }

  return success;
}

int testNotOperator(void) {
  {
    char *script = "!hello";
    lexAndParse(script);
    if (
      nodes[0]->type == NODE_BINOP &&
      nodes[0]->content.binopNode->type == BINOP_NOT &&
      nodes[0]->content.binopNode->LHS == NULL &&
      nodes[0]->content.binopNode->RHS->type == NODE_IDENTIFIER
    ) {
      return 1;
    } else {
      printf("Failed test because of %s\n", script); \
      abort();
    }
  }
  {
    char *script = "~hello";
    lexAndParse(script);
    if (
      nodes[0]->type == NODE_BINOP &&
      nodes[0]->content.binopNode->type == BINOP_BNOT &&
      nodes[0]->content.binopNode->LHS == NULL &&
      nodes[0]->content.binopNode->RHS->type == NODE_IDENTIFIER
    ) {
      return 1;
    } else {
      printf("Failed test because of %s\n", script); \
      abort();
    }
  }
}

int testFunctionNode(void) {
  // () -> { # the body may contain anything # }
  srand(time(NULL));
  uint8_t success = 1;

  for (int tries = 100; tries > 0; tries--) {
    int parameterCount = rand() % 6;
    int totalScriptSize = 7;
    char **parameters = malloc(parameterCount * sizeof(char *));
    for (int i = 0; i < parameterCount; i++) {
      generateRandomIdentifier(parameters[i], (rand() % 30) + 1);
      totalScriptSize += strlen(parameters[i]) + 1; // include ,
    }
    char *statements[] = { "value = 20;", "hello = () -> { 10; };", "printf(\"%s\", \"Hello, world!\");", "value = 6 + 20 * 90;" };
    int statementCount = rand() % 10;
    char **functionBody = malloc(statementCount * sizeof(char *));
    for (int i = 0; i < statementCount; i++) {
      functionBody[i] = statements[rand() % (sizeof(statements) / sizeof(statements[0]))];
      totalScriptSize += strlen(functionBody[i]);
    }
    char *script = malloc(totalScriptSize * sizeof(char));
    strcpy(script, "(");
    for (int i = 0; i < parameterCount; i++) {
      strcat(script, parameters[i]);
      if (i < parameterCount - 1) strcat(script, ",");
    }
    strcat(script, ")->{");
    for (int i = 0; i < statementCount; i++) strcat(script, statements[i]);
    strcat(script, "}");
    lexAndParse(script);
    success = success && (nodes[0]->type == NODE_FUNCTION && nodes[0]->content.functionNode->bodyCount == statementCount && nodes[0]->content.functionNode->paramsCount == parameterCount);
    abortOnFail(script)
    free(script); freeArray(statements, statementCount);
    freeArray(parameters, parameterCount);
    freeArray(tokens, tokensLength); freeArray(nodes, nodesLength);
  }
  
  return success;
}

int testFunctionCall(void) {
  srand(time(NULL));
  uint8_t success = 1;
  for (int tries = 100; tries > 0; tries--) {
    char *identifier = NULL;
    generateRandomIdentifier(identifier, (rand() % 30) + 1);
    int argCount = rand() % 6;
    char **args = malloc(argCount * sizeof(char *));
    int totalScriptLength = 4 + strlen(identifier);
    for (int i = 0; i < argCount; i++) {
      generateRandomIdentifier(args[i], (rand() % 30) + 1);
      totalScriptLength += strlen(args[i]) + 1;
    }
    char *script = malloc(totalScriptLength * sizeof(char));
    strcpy(script, identifier); strcat(script, "(");
    for (int i = 0; i < argCount; i++) {
      strcat(script, args[i]);
      if (i < argCount - 1) strcat(script, ",");
    }
    strcat(script, ");");
    lexAndParse(script);
    success = success && (nodesLength == 1 &&
      nodes[0]->type == NODE_FUNCTIONCALL &&
      strcmp(nodes[0]->content.functionCallNode->function->content.identifierNode->name, identifier) == 0
    );
    abortOnFail(script)
    freeArray(tokens, tokensLength);
    freeArray(nodes, nodesLength);
    free(script); freeArray(args, argCount); free(identifier);
  }

  return success;
}

int testIdentifierNode(void) {
  srand(time(NULL));
  uint8_t success = 1;

  for (int tries = 100; tries > 0; tries--) {
    char *identifier = NULL;
    generateRandomIdentifier(identifier, (rand() % 30) + 1);
    char *script = malloc(strlen(identifier) + 2);
    strcpy(script, identifier); strcat(script, ";");
    lexAndParse(script);
    success = success && (nodes[0]->type == NODE_IDENTIFIER && strcmp(nodes[0]->content.identifierNode->name, identifier) == 0);
    abortOnFail(script)
    freeArray(tokens, tokensLength); freeArray(nodes, nodesLength);
    free(script); free(identifier);
  }

  return success;
}

int testNumberNode(void) {
  srand(time(NULL));
  uint8_t success = 1;
  for (int tries = 100; tries > 0; tries--) {
    generateRandomNumber(number);
    char *script = calloc(14, sizeof(char));
    strcpy(script, number);
    strcat(script, ";");
    lexAndParse(number);
    success = success && (nodes[0]->type == NODE_NUMBER && nodes[0]->content.numberNode->value == randomNum);
    abortOnFail(number)
  }

  return success;
}

int testStringLiteralNode(void) {
  srand(time(NULL));
  uint8_t success = 1;

  for (int tries = 100; tries > 0; tries--) {
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
    lexAndParse(string);
    char *filteredString = calloc(length, sizeof(char));
    strcpy(filteredString, string + 1);
    filteredString[length - 2] = '\0';
    success = success && (nodes[0]->type == NODE_STRINGLITERAL && strcmp(nodes[0]->content.stringLiteralNode->value, filteredString) == 0);
    abortOnFail(string);
    free(filteredString); free(string); freeArray(tokens, tokensLength); freeArray(nodes, nodesLength);
  }

  return success;
}

int testValueAtOperator(void) {
  // Test 3 situtations for the value at operator
  uint8_t success = 1;
  {
    char *script = "*string = \"Hello\";";
    lexAndParse(script);
    success = success && (
      nodes[0]->type == NODE_BINOP &&
      nodes[0]->content.binopNode->type == BINOP_ASGN &&
      nodes[0]->content.binopNode->LHS->type == NODE_BINOP &&
      nodes[0]->content.binopNode->LHS->content.binopNode->type == BINOP_VAL &&
      nodes[0]->content.binopNode->LHS->content.binopNode->LHS == NULL &&
      nodes[0]->content.binopNode->LHS->content.binopNode->RHS->type == NODE_IDENTIFIER
    );
    abortOnFail(script);
    freeArray(tokens, tokensLength); freeArray(nodes, nodesLength);
  }
  {
    char *script = "*(song + 8) = 10;";
    lexAndParse(script);
    success = success && (
      nodes[0]->type == NODE_BINOP &&
      nodes[0]->content.binopNode->type == BINOP_ASGN &&
      nodes[0]->content.binopNode->LHS->type == NODE_BINOP &&
      nodes[0]->content.binopNode->LHS->content.binopNode->type == BINOP_VAL &&
      nodes[0]->content.binopNode->LHS->content.binopNode->LHS == NULL &&
      nodes[0]->content.binopNode->LHS->content.binopNode->RHS->type == NODE_BINOP &&
      nodes[0]->content.binopNode->LHS->content.binopNode->RHS->content.binopNode->type == BINOP_ADD
    );
    abortOnFail(script);
    freeArray(tokens, tokensLength); freeArray(nodes, nodesLength);
  }
  {
    char *script = "value = 10 * *hello;";
    lexAndParse(script);
    success = success && (
      nodes[0]->type == NODE_BINOP &&
      nodes[0]->content.binopNode->type == BINOP_ASGN &&
      nodes[0]->content.binopNode->RHS->type == NODE_BINOP &&
      nodes[0]->content.binopNode->RHS->content.binopNode->type == BINOP_MUL &&
      nodes[0]->content.binopNode->RHS->content.binopNode->LHS->type == NODE_NUMBER &&
      nodes[0]->content.binopNode->RHS->content.binopNode->RHS->type == NODE_BINOP &&
      nodes[0]->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->type == BINOP_VAL
    );
    abortOnFail(script);
    freeArray(tokens, tokensLength); freeArray(nodes, nodesLength);
  }

  return success;
}

int testExpressions(void) {
  uint8_t success = 1;

  {
    char *script = "20 * 50 + 60;";
    lexAndParse(script);
    success = success && (
      nodes[0]->content.binopNode->type == BINOP_ADD &&
      nodes[0]->content.binopNode->LHS->type == NODE_BINOP &&
      nodes[0]->content.binopNode->LHS->content.binopNode->type == BINOP_MUL
    );
    abortOnFail(script);
  }
  {
    char *script = "10 * (5 + 5);";
    lexAndParse(script);
    success = success && (
      nodes[0]->content.binopNode->type == BINOP_MUL &&
      nodes[0]->content.binopNode->RHS->type == NODE_BINOP &&
      nodes[0]->content.binopNode->RHS->content.binopNode->type == BINOP_ADD
    );
    abortOnFail(script);
  }
  {
    char *script = "10 + hello();";
    lexAndParse(script);
    success = success && (
      nodes[0]->content.binopNode->type == BINOP_ADD &&
      nodes[0]->content.binopNode->LHS->type == NODE_NUMBER &&
      nodes[0]->content.binopNode->RHS->type == NODE_FUNCTIONCALL &&
      nodes[0]->content.binopNode->RHS->content.functionCallNode->function->type == NODE_IDENTIFIER
    );
    abortOnFail(script);
  }
  {
    char *script = "hello = () -> {};";
    lexAndParse(script);
    success = success && (
      nodes[0]->content.binopNode->type == BINOP_ASGN &&
      nodes[0]->content.binopNode->LHS->type == NODE_IDENTIFIER &&
      nodes[0]->content.binopNode->RHS->type == NODE_FUNCTION
    );
    abortOnFail(script);
  }
  {
    char *script = "() -> { printf(\"Hello\"); }();";
    lexAndParse(script);
    success = success && (
      nodes[0]->type == NODE_FUNCTIONCALL &&
      nodes[0]->content.functionCallNode->function->type == NODE_FUNCTION
    );
    abortOnFail(script);
  }
  {
    char *script = "if(condition == true, () -> { printf(\"Hello\") });";
    lexAndParse(script);
    success = success && (
      nodes[0]->type == NODE_FUNCTIONCALL &&
      nodes[0]->content.functionCallNode->function->type == NODE_IDENTIFIER &&
      nodes[0]->content.functionCallNode->argsCount == 2 &&
      nodes[0]->content.functionCallNode->args[0]->type == NODE_BINOP &&
      nodes[0]->content.functionCallNode->args[1]->type == NODE_FUNCTION
    );
    abortOnFail(script);
  }
  {
    char *script = "run(hello(world));";
    lexAndParse(script);
    success = success && (
      nodes[0]->type == NODE_FUNCTIONCALL &&
      nodes[0]->content.functionCallNode->function->type == NODE_IDENTIFIER &&
      nodes[0]->content.functionCallNode->args[0]->type == NODE_FUNCTIONCALL &&
      nodes[0]->content.functionCallNode->args[0]->content.functionCallNode->args[0]->type == NODE_IDENTIFIER
    );
    abortOnFail(script);
  }
  {
    char *script = "run(hello + world, how - are - you);";
    lexAndParse(script);
    success = success && (
      nodes[0]->type == NODE_FUNCTIONCALL &&
      nodes[0]->content.functionCallNode->function->type == NODE_IDENTIFIER &&
      nodes[0]->content.functionCallNode->args[0]->type == NODE_BINOP &&
      nodes[0]->content.functionCallNode->args[0]->content.binopNode->type == BINOP_ADD &&
      nodes[0]->content.functionCallNode->args[1]->type == NODE_BINOP &&
      nodes[0]->content.functionCallNode->args[1]->content.binopNode->type == BINOP_SUB
    );
    abortOnFail(script);
  }
  {
    char *script = "returnFunction(\"Hello\")();";
    lexAndParse(script);
    success = success && (
      nodes[0]->type == NODE_FUNCTIONCALL &&
      nodes[0]->content.functionCallNode->function->type == NODE_FUNCTIONCALL &&
      nodes[0]->content.functionCallNode->function->content.functionCallNode->function->type == NODE_IDENTIFIER &&
      nodes[0]->content.functionCallNode->function->content.functionCallNode->argsCount == 1 
    );
    abortOnFail(script);
  }
  return success;
}

int testOperatorPresedence(void) {
  uint8_t success = 1;
  {
    char *script = "hello = 10 + 50 << 1 * 4 - 30 >> 5 / 8;";
    lexAndParse(script);
    success = success && (
      nodes[0]->content.binopNode->type == BINOP_ASGN && // lowest presedence
      nodes[0]->content.binopNode->RHS->content.binopNode->type == BINOP_BSL &&
      nodes[0]->content.binopNode->RHS->content.binopNode->LHS->content.binopNode->type == BINOP_ADD &&
      nodes[0]->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->type == BINOP_BSR && 
      nodes[0]->content.binopNode->RHS->content.binopNode->LHS->content.binopNode->LHS->content.binopNode->type == BINOP_SUB &&
      nodes[0]->content.binopNode->RHS->content.binopNode->LHS->content.binopNode->RHS->content.binopNode->type == BINOP_DIV &&
      nodes[0]->content.binopNode->RHS->content.binopNode->LHS->content.binopNode->LHS->content.binopNode->LHS->content.binopNode->type == BINOP_MUL 
    );
    abortOnFail(script);
    freeArray(nodes, nodesLength); freeArray(tokens, tokensLength);
  }
  {
    char *script = "10 < 20 || 20 >= 9 && 10 <= 20 && 9 > !14;";
    lexAndParse(script);
    success = success && (
      nodes[0]->content.binopNode->type == BINOP_OR && 
      nodes[0]->content.binopNode->LHS->content.binopNode->type == BINOP_LT &&
      nodes[0]->content.binopNode->RHS->content.binopNode->type == BINOP_AND &&
      nodes[0]->content.binopNode->RHS->content.binopNode->LHS->content.binopNode->type == BINOP_GTE &&
      nodes[0]->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->type == BINOP_AND &&
      nodes[0]->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->LHS->content.binopNode->type == BINOP_LTE && 
      nodes[0]->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->type == BINOP_GT &&
      nodes[0]->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->type == BINOP_NOT && 
      nodes[0]->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->LHS == NULL 
    );
    abortOnFail(script);
    freeArray(nodes, nodesLength); freeArray(tokens, tokensLength);
  }
  {
    char *script = "100 & 1 + 20 | 2 + 20 ^ 1 + ~1;";
    lexAndParse(script);
    success = success && (
      nodes[0]->content.binopNode->type == BINOP_BOR &&
      nodes[0]->content.binopNode->LHS->content.binopNode->type == BINOP_BAND &&
      nodes[0]->content.binopNode->LHS->content.binopNode->RHS->content.binopNode->type == BINOP_ADD &&
      nodes[0]->content.binopNode->RHS->content.binopNode->LHS->content.binopNode->type == BINOP_ADD &&
      nodes[0]->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->type == BINOP_ADD &&
      nodes[0]->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->type == BINOP_BNOT
    );
    abortOnFail(script);
    freeArray(nodes, nodesLength); freeArray(tokens, tokensLength);
  }

  return success;
}

TestCase tests[] = {
  { testBinopNode, "binary operators are correctly parsed" },
  { testNotOperator, "not operators are correctly parsed" },
  { testFunctionNode, "function definitions are correctly parsed" },
  { testFunctionCall, "function calls are correctly parsed" },
  { testIdentifierNode, "identifiers are correctly parsed" },
  { testNumberNode, "numbers are correctly parsed" },
  { testStringLiteralNode, "string literals are correctly parsed" },
  { testValueAtOperator, "value at operator is correctly parsed" },
  { testExpressions, "Common expressions are correctly parsed" },
  { testOperatorPresedence, "Operator presedence is taken into account" }
};

int main(void) {
  runTests(tests);
}