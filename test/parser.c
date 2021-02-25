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

#define parseNextExpression() parseNextExpression(";")

// TODO: Make freeNode function for correctly freeing nodes 

int testBinopNode(void) {
  uint8_t success = 1;

  for (int tries = 100; tries > 0; tries--) {
    char *binops[] = {"=", "+", "-", "/", "*", "<", ">", "<=", ">=", "||", "&&", "|", "&", "<<", ">>", "==", "!="};
    srand(time(NULL));
    uint8_t isIdentifier = rand() % 2 == 0;
    char *operands[2] = { NULL, NULL };
    for (int i = 0; i < 2; i++) {
      char *identifier = NULL;
      generateRandomIdentifier(identifier, (rand() % 30) + 1);
      operands[i] = identifier;
    }
    char *randomBinop = binops[rand() % (sizeof(binops) / sizeof(binops[0]))];
    char *script = malloc(strlen(randomBinop) + strlen(operands[0]) + strlen(operands[1]) + 2);
    strcpy(script, operands[0]); strcat(script, randomBinop); strcat(script, operands[1]);
    strcat(script, ";");
    setScript(script);
    Node *expression = parseNextExpression();

    if (
      expression->type == NODE_BINOP &&
      strcmp(expression->content.binopNode->LHS->content.identifierNode->name, operands[0]) == 0 &&
      strcmp(expression->content.binopNode->RHS->content.identifierNode->name, operands[1]) == 0
    ) {
      success = 1;
    } else {
      success = 0;
      break;
    }
    free(expression);
    free(operands[0]); free(operands[1]);
    free(script);
  }

  return success;
}

int testNotOperator(void) {
  {
    char *script = "!hello;";
    setScript(script);
    Node *expression = parseNextExpression();
    if (
      expression->type == NODE_BINOP &&
      expression->content.binopNode->type == BINOP_NOT &&
      expression->content.binopNode->LHS == NULL &&
      expression->content.binopNode->RHS->type == NODE_IDENTIFIER
    ) {
      return 1;
    } else {
      printf("Failed test because of %s\n", script); \
      abort();
    }
    free(expression);
  }
  {
    char *script = "~hello;";
    setScript(script);
    Node *expression = parseNextExpression();
    if (
      expression->type == NODE_BINOP &&
      expression->content.binopNode->type == BINOP_BNOT &&
      expression->content.binopNode->LHS == NULL &&
      expression->content.binopNode->RHS->type == NODE_IDENTIFIER
    ) {
      return 1;
    } else {
      printf("Failed test because of %s\n", script); \
      abort();
    }
    free(expression);
  }
}

int testFunctionNode(void) {
  // () -> { # the body may contain anything # }
  srand(time(NULL));
  uint8_t success = 1;

  for (int tries = 100; tries > 0; tries--) {
    int parameterCount = rand() % 6;
    int totalScriptSize = 10;
    char **parameters = malloc(parameterCount * sizeof(char *));
    for (int i = 0; i < parameterCount; i++) {
      char *identifier = NULL;
      generateRandomIdentifier(identifier, (rand() % 30) + 1);
      parameters[i] = identifier;
      totalScriptSize += strlen(parameters[i]) + 1; // include ,
    }
    char *statements[] = { "value = 20;", "printf(\"%s\", \"Hello, world!\");", "value = 6 + 20 * 90;" };
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
    for (int i = 0; i < statementCount; i++) strcat(script, functionBody[i]);
    const char *ending = "};";
    strcat(script, ending);
    setScript(script);
    Node *expression = parseNextExpression();
    success = success && (expression->type == NODE_FUNCTION && expression->content.functionNode->bodyCount == statementCount && expression->content.functionNode->paramsCount == parameterCount);
    abortOnFail(script)
    free(script); free(functionBody);
    freeArray(parameters, parameterCount);
    free(expression);
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
      char *identifier = NULL;
      generateRandomIdentifier(identifier, (rand() % 30) + 1);
      args[i] = identifier;
      totalScriptLength += strlen(args[i]) + 1;
    }
    char *script = malloc(totalScriptLength * sizeof(char));
    strcpy(script, identifier); strcat(script, "(");
    for (int i = 0; i < argCount; i++) {
      strcat(script, args[i]);
      if (i < argCount - 1) strcat(script, ",");
    }
    strcat(script, ");");
    setScript(script);
    Node *expression = parseNextExpression();
    success = success && (
      expression->type == NODE_FUNCTIONCALL &&
      strcmp(expression->content.functionCallNode->function->content.identifierNode->name, identifier) == 0
    );
    abortOnFail(script)
    free(expression);
    free(script); freeArray(args, argCount); free(identifier);
  }

  return success;
}

int testIdentifierNode(void) {
  srand(time(NULL));
  uint8_t success = 1;

  for (int tries = 100; tries > 0; tries--) {
    char *identifier = NULL;
    int length = (rand() % 30) + 2;
    identifier = calloc(length, sizeof(char)); \
    char character = (rand() % 26) + 'A'; \
    identifier[0] = character; \
    for (int i = 1; i < length - 1; i++) { \
      uint8_t type = rand() % 4; \
      char character; \
      switch (type) { \
        case 0: character = rand() % 9 + '0'; break; \
        case 1: character = rand() % 25 + 'A'; break; \
        case 2: character = rand() % 25 + 'a'; break; \
        case 3: character = '_'; break; \
      } \
      identifier[i] = character; \
    }
    char *script = malloc(strlen(identifier) + 2);
    strcpy(script, identifier); strcat(script, ";");
    setScript(script);
    Node *expression = parseNextExpression();
    success = success && (expression->type == NODE_IDENTIFIER && strcmp(expression->content.identifierNode->name, identifier) == 0);
    abortOnFail(script)
    free(expression);
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
    setScript(script);
    Node *expression = parseNextExpression();
    success = success && (expression->type == NODE_NUMBER && expression->content.numberNode->value == randomNum);
    abortOnFail(number)
    free(script);
    free(expression);
  }

  return success;
}

int testStringLiteralNode(void) {
  srand(time(NULL));
  uint8_t success = 1;

  for (int tries = 100; tries > 0; tries--) {
    int length = (rand() % 30) + 2;
    char *string = calloc(length + 2, sizeof(char));
    string[0] = '"';
    for (int i = 1; i < length - 1; i++) {
      char character;
      do {
        character = rand() % ('~' - ' ') + ' ';
        string[i] = character;
      } while (character == '"' || character == '\\');
    }
    strcat(string, "\";");
    setScript(string);
    Node *expression = parseNextExpression();
    char *filteredString = calloc(length, sizeof(char));
    strcpy(filteredString, string + 1);
    filteredString[length - 2] = '\0';
    success = success && (expression->type == NODE_STRINGLITERAL && strcmp(expression->content.stringLiteralNode->value, filteredString) == 0);
    abortOnFail(string);
    free(filteredString); free(string); free(expression);
  }

  return success;
}

int testValueAtOperator(void) {
  // Test 3 situtations for the value at operator
  uint8_t success = 1;
  {
    char *script = "*string = \"Hello\";";
    setScript(script);
    Node *expression = parseNextExpression();
    success = success && (
      expression->type == NODE_BINOP &&
      expression->content.binopNode->type == BINOP_ASGN &&
      expression->content.binopNode->LHS->type == NODE_BINOP &&
      expression->content.binopNode->LHS->content.binopNode->type == BINOP_VAL &&
      expression->content.binopNode->LHS->content.binopNode->LHS == NULL &&
      expression->content.binopNode->LHS->content.binopNode->RHS->type == NODE_IDENTIFIER
    );
    abortOnFail(script);
    free(expression);
  }
  {
    char *script = "*(song + 8) = 10;";
    setScript(script);
    Node *expression = parseNextExpression();
    success = success && (
      expression->type == NODE_BINOP &&
      expression->content.binopNode->type == BINOP_ASGN &&
      expression->content.binopNode->LHS->type == NODE_BINOP &&
      expression->content.binopNode->LHS->content.binopNode->type == BINOP_VAL &&
      expression->content.binopNode->LHS->content.binopNode->LHS == NULL &&
      expression->content.binopNode->LHS->content.binopNode->RHS->type == NODE_BINOP &&
      expression->content.binopNode->LHS->content.binopNode->RHS->content.binopNode->type == BINOP_ADD
    );
    abortOnFail(script);
    free(expression);
  }
  {
    char *script = "value = 10 * *hello;";
    setScript(script);
    Node *expression = parseNextExpression();
    success = success && (
      expression->type == NODE_BINOP &&
      expression->content.binopNode->type == BINOP_ASGN &&
      expression->content.binopNode->RHS->type == NODE_BINOP &&
      expression->content.binopNode->RHS->content.binopNode->type == BINOP_MUL &&
      expression->content.binopNode->RHS->content.binopNode->LHS->type == NODE_NUMBER &&
      expression->content.binopNode->RHS->content.binopNode->RHS->type == NODE_BINOP &&
      expression->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->type == BINOP_VAL
    );
    abortOnFail(script);
    free(expression);
  }

  return success;
}

int testExpressions(void) {
  uint8_t success = 1;

  {
    char *script = "20 * 50 + 60;";
    setScript(script);
    Node *expression = parseNextExpression();
    success = success && (
      expression->content.binopNode->type == BINOP_ADD &&
      expression->content.binopNode->LHS->type == NODE_BINOP &&
      expression->content.binopNode->LHS->content.binopNode->type == BINOP_MUL
    );
    abortOnFail(script);
    free(expression);
  }
  {
    char *script = "10 * (5 + 5);";
    setScript(script);
    Node *expression = parseNextExpression();
    success = success && (
      expression->content.binopNode->type == BINOP_MUL &&
      expression->content.binopNode->RHS->type == NODE_BINOP &&
      expression->content.binopNode->RHS->content.binopNode->type == BINOP_ADD
    );
    abortOnFail(script);
    free(expression);
  }
  {
    char *script = "10 + hello();";
    setScript(script);
    Node *expression = parseNextExpression();
    success = success && (
      expression->content.binopNode->type == BINOP_ADD &&
      expression->content.binopNode->LHS->type == NODE_NUMBER &&
      expression->content.binopNode->RHS->type == NODE_FUNCTIONCALL &&
      expression->content.binopNode->RHS->content.functionCallNode->function->type == NODE_IDENTIFIER
    );
    abortOnFail(script);
    free(expression);
  }
  {
    char *script = "hello = () -> {};";
    setScript(script);
    Node *expression = parseNextExpression();
    success = success && (
      expression->content.binopNode->type == BINOP_ASGN &&
      expression->content.binopNode->LHS->type == NODE_IDENTIFIER &&
      expression->content.binopNode->RHS->type == NODE_FUNCTION
    );
    abortOnFail(script);
    free(expression);
  }
  {
    char *script = "() -> { printf(\"Hello\"); }();";
    setScript(script);
    Node *expression = parseNextExpression();
    success = success && (
      expression->type == NODE_FUNCTIONCALL &&
      expression->content.functionCallNode->function->type == NODE_FUNCTION
    );
    abortOnFail(script);
    free(expression);
  }
  {
    char *script = "if(condition == true, () -> { printf(\"Hello\"); });";
    setScript(script);
    Node *expression = parseNextExpression();
    success = success && (
      expression->type == NODE_FUNCTIONCALL &&
      expression->content.functionCallNode->function->type == NODE_IDENTIFIER &&
      expression->content.functionCallNode->argsCount == 2 &&
      expression->content.functionCallNode->args[0]->type == NODE_BINOP &&
      expression->content.functionCallNode->args[1]->type == NODE_FUNCTION
    );
    abortOnFail(script);
    free(expression);
  }
  {
    char *script = "run(hello(world));";
    setScript(script);
    Node *expression = parseNextExpression();
    success = success && (
      expression->type == NODE_FUNCTIONCALL &&
      expression->content.functionCallNode->function->type == NODE_IDENTIFIER &&
      expression->content.functionCallNode->args[0]->type == NODE_FUNCTIONCALL &&
      expression->content.functionCallNode->args[0]->content.functionCallNode->args[0]->type == NODE_IDENTIFIER
    );
    abortOnFail(script);
    free(expression);
  }
  {
    char *script = "run(hello + world, how - are - you);";
    setScript(script);
    Node *expression = parseNextExpression();
    success = success && (
      expression->type == NODE_FUNCTIONCALL &&
      expression->content.functionCallNode->function->type == NODE_IDENTIFIER &&
      expression->content.functionCallNode->args[0]->type == NODE_BINOP &&
      expression->content.functionCallNode->args[0]->content.binopNode->type == BINOP_ADD &&
      expression->content.functionCallNode->args[1]->type == NODE_BINOP &&
      expression->content.functionCallNode->args[1]->content.binopNode->type == BINOP_SUB
    );
    abortOnFail(script);
    free(expression);
  }
  {
    char *script = "returnFunction(\"Hello\")();";
    setScript(script);
    Node *expression = parseNextExpression();
    success = success && (
      expression->type == NODE_FUNCTIONCALL &&
      expression->content.functionCallNode->function->type == NODE_FUNCTIONCALL &&
      expression->content.functionCallNode->function->content.functionCallNode->function->type == NODE_IDENTIFIER &&
      expression->content.functionCallNode->function->content.functionCallNode->argsCount == 1 
    );
    abortOnFail(script);
    free(expression);
  }
  return success;
}

int testOperatorPresedence(void) {
  uint8_t success = 1;
  {
    char *script = "hello = 10 + 50 << 1 * 4 - 30 >> 5 / 8;";
    setScript(script);
    Node *expression = parseNextExpression();
    success = success && (
      expression->content.binopNode->type == BINOP_ASGN && // lowest presedence
      expression->content.binopNode->RHS->content.binopNode->type == BINOP_BSL &&
      expression->content.binopNode->RHS->content.binopNode->LHS->content.binopNode->type == BINOP_ADD &&
      expression->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->type == BINOP_BSR && 
      expression->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->LHS->content.binopNode->type == BINOP_SUB &&
      expression->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->type == BINOP_DIV &&
      expression->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->LHS->content.binopNode->LHS->content.binopNode->type == BINOP_MUL 
    );
    abortOnFail(script);
    free(expression);
  }
  {
    char *script = "10 < 20 || 20 >= 9 && 10 <= 20 && 9 > !14;";
    setScript(script);
    Node *expression = parseNextExpression();
    success = success && (
      expression->content.binopNode->type == BINOP_OR && 
      expression->content.binopNode->LHS->content.binopNode->type == BINOP_LT &&
      expression->content.binopNode->RHS->content.binopNode->type == BINOP_AND &&
      expression->content.binopNode->RHS->content.binopNode->LHS->content.binopNode->type == BINOP_GTE &&
      expression->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->type == BINOP_AND &&
      expression->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->LHS->content.binopNode->type == BINOP_LTE && 
      expression->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->type == BINOP_GT &&
      expression->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->type == BINOP_NOT && 
      expression->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->LHS == NULL 
    );
    abortOnFail(script);
    free(expression);
  }
  {
    char *script = "100 & 1 + 20 | 2 + ~1;";
    setScript(script);
    Node *expression = parseNextExpression();
    success = success && (
      expression->content.binopNode->type == BINOP_BOR &&
      expression->content.binopNode->LHS->content.binopNode->type == BINOP_BAND &&
      expression->content.binopNode->LHS->content.binopNode->RHS->content.binopNode->type == BINOP_ADD &&
      expression->content.binopNode->RHS->content.binopNode->type == BINOP_ADD &&
      expression->content.binopNode->RHS->content.binopNode->RHS->content.binopNode->type == BINOP_BNOT
    );
    abortOnFail(script);
    free(expression);
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
