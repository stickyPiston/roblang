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
  NODE_NONE
} NodeType;

typedef enum {
  CHUNK_BINOP,
  CHUNK_FUNCTION,
  CHUNK_FUNCTIONCALL,
  CHUNK_IDENTIFIER,
  CHUNK_NUMBER,
  CHUNK_STRINGLITERAL,
  CHUNK_BRACKET,
  CHUNK_DELIMITER,
  CHUNK_EXPRESSION,
  CHUNK_NONE
} ChunkType;

struct Node;
struct Chunk;

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
  BINOP_BNOT, // ~
  BINOP_NONE
} BinopType;

typedef struct {
  BinopType type;
} BinopChunk;

typedef struct {
  char *name;
} IdentifierChunk;

typedef struct {
  int value;
} NumberChunk;

typedef struct {
  char *value;
} StringLiteralChunk;

typedef struct {
  char bracket;
} BracketChunk;

typedef struct {
  char delimiter;
} DelimiterChunk;

typedef struct {
  char **params;
  int paramsCount;
  struct Node **body;
  int bodyLength;
} FunctionChunk;

typedef struct {
  struct Node **args;
  int argsCount;
  struct Chunk *callee;
} FunctionCallChunk;

typedef struct {
  struct Node *expression;
} ExpressionChunk;

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

typedef struct Chunk {
  int col;
  int row;
  ChunkType type;
  union {
    BinopChunk          *binopChunk;
    IdentifierChunk     *identifierChunk;
    NumberChunk         *numberChunk;
    StringLiteralChunk  *stringLiteralChunk;
    BracketChunk        *bracketChunk;
    DelimiterChunk      *delimiterChunk;
    FunctionChunk       *functionChunk;
    FunctionCallChunk   *functionCallChunk;
    ExpressionChunk     *expressionChunk;
  } content;
} Chunk;

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
  } content;
} Node;

Chunk *parseNextChunk();
Node *parseNextExpression(char *delimiter);
Node *arrangeChunksIntoNode(Chunk **, int);

#endif
