#ifndef ROBLANG_CODEGEN_H
#define ROBLANG_CODEGEN_H

#include <roblang/parser.h>

#include <llvm-c/Core.h>

/* void codegenNext(Node *node); */
LLVMValueRef codegenBinop(Node *);
LLVMValueRef codegenFunctionCall(Node *);
LLVMValueRef codegenNumber(Node *);
LLVMValueRef codegenNext(Node *);
LLVMValueRef codegenFunction(Node *, const char *);
LLVMValueRef codegenIdentifier(Node *);

#endif
