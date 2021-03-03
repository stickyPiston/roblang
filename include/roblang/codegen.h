#ifndef ROBLANG_CODEGEN_H
#define ROBLANG_CODEGEN_H

#include <roblang/parser.h>

typedef enum {
  REGISTER_EAX
} Register;

/* void codegenNext(Node *node); */
void codegenBinop(Node *node);
void codegenVariable(Node *node, Register);
void codegenFunction(Node *node);

#endif
