#ifndef ROBLANG_PARSER_H
#define ROBLANG_PARSER_H

#include <roblang/lexer.h>

// Every statement in roblang is an expression

typedef enum {
  NODE_BINOP,
  NODE_FUNCTION,
  NODE_FUNCTIONCALL,
  NODE_IDENTIFIER,
  NODE_NUMBER,
  NODE_STRINGLITERAL,
  NODE_BRACKET,
  NODE_NONE
} NodeType;

struct Node;

typedef enum {
  BINOP_ASGN, // =
  BINOP_ADD,  // +
  BINOP_SUB,  // -
  BINOP_MUL,  // *
  BINOP_DIV,  // /
  BINOP_GT,   // >
  BINOP_GTE,  // >=
  BINOP_LT,   // <
  BINOP_LTE,  // <=
  BINOP_AND,  // &&
  BINOP_BAND, // &
  BINOP_OR,   // ||
  BINOP_BOR,  // |
  BINOP_BSR,  // >>
  BINOP_BSL,  // <<
  BINOP_EQ,   // ==
  BINOP_NEQ,  // !=
  BINOP_NOT,  // !
  BINOP_VAL,  // *string
  BINOP_BNOT  // ~
} BinopType;

typedef struct {
  struct Node *LHS;
  struct Node *RHS;
  BinopType type;
} BinopNode;

typedef struct {
  char **params;
  int paramsCount;
  struct Node **body;
  int bodyCount;
} FunctionNode;

typedef struct {
  struct Node *function;
  struct Node **args;
  int argsCount;
} FunctionCallNode;

typedef struct {
  char *name;
} IdentifierNode;

typedef struct {
  int value;
} NumberNode;

typedef struct {
  char *value;
} StringLiteralNode;

typedef struct {
  char bracket;
} BracketNode;

typedef struct Node {
  int col;
  int row;
  NodeType type;
  union {
    BinopNode         *binopNode;
    FunctionNode      *functionNode;
    FunctionCallNode  *functionCallNode;
    IdentifierNode    *identifierNode;
    NumberNode        *numberNode;
    StringLiteralNode *stringLiteralNode;
    BracketNode       *bracketNode;
  } content;
} Node;

Node *parseNextNode();
Node *parseNextExpression();

#endif
