#include <roblang/parser.h>
#include <roblang/codegen.h>
#include <roblang/file.h>

#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>

#include <stdlib.h>
#include <stdio.h>

LLVMBuilderRef builder = NULL;
LLVMModuleRef module = NULL;

int main(int argc, char **argv) {
  if (argc < 2) return 1;

  setScript(readFile(argv[1]));

  // Set up LLVM bits
  module = LLVMModuleCreateWithName("main");
  builder = LLVMCreateBuilder();

  while (1) {
    Node *node = parseNextExpression(";");
    if (node == NULL) break;
    codegenNext(node);
  }

  // Output object file
  char *error = NULL;
  LLVMVerifyModule(module, LLVMAbortProcessAction, &error);
  LLVMDisposeMessage(error);

  if (LLVMWriteBitcodeToFile(module, "out.bc") != 0) {
    printf("Error writing bitcode to file\n");
    abort();
  } else {
    printf("Written bitcode to out.bc!\n");
  } 

  return 0;
}
