#ifndef ROBLANG_CODEGEN_H
#define ROBLANG_CODEGEN_H

#include <roblang/stack.h>
#include <roblang/parser.h>

typedef enum {
  REGISTER_EAX,
  REGISTER_EBX,
  REGISTER_ECX,
  REGISTER_EDX,
  REGISTER_EDI,
  REGISTER_ESI,
  REGISTER_R8D,
  REGISTER_R9D,
  REGISTER_NONE
} Register;

/* void codegenNext(Node *node); */
void codegenBinop(Node *node);
Variable *codegenVariable(Node *node, Register);
void codegenArguments(FunctionCallNode *node);
void codegenFunctionCall(Node *node);
void codegenNumber(Node *node, Register r);
void codegenNextTop(Node *node);
void codegenFunction(Node *node, char *);

#endif
