#ifndef ROBLANG_CODEGEN_H
#define ROBLANG_CODEGEN_H

#include <roblang/stack.h>
#include <roblang/parser.h>

typedef enum {
  REGISTER_EAX,
  REGISTER_EBX,
  REGISTER_ECX,
  REGISTER_NONE
} Register;

/* void codegenNext(Node *node); */
void codegenBinop(Node *node);
Variable *codegenVariable(Node *node, Register);
void codegenFunction(Node *node, char *);

#endif
