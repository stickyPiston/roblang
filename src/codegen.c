#include <roblang/codegen.h>
#include <roblang/variableCache.h>

#include <llvm-c/Core.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

extern LLVMBuilderRef builder;
extern LLVMModuleRef  module;

static LLVMValueRef currentFunc      = NULL;
static bool         isLastStatement  = 0;

LLVMValueRef codegenNext(Node *node) {
  switch (node->type) {
    case NODE_BINOP: return codegenBinop(node);
    case NODE_FUNCTION: return codegenFunction(node, NULL);
    case NODE_FUNCTIONCALL: return codegenFunctionCall(node);
    case NODE_IDENTIFIER: return codegenIdentifier(node);
    case NODE_NUMBER: return codegenNumber(node);
    default: return NULL;
  }
}

LLVMValueRef codegenFunction(Node *node, const char *name) {
  LLVMTypeRef ret_type = LLVMInt32Type();
  LLVMTypeRef *param_types = NULL;
  int paramCount = node->content.functionNode->paramsCount;
  if (paramCount > 0) {
    param_types = malloc(sizeof(LLVMTypeRef) * paramCount);
    for (int i = 0; i < paramCount; i++)
      param_types[i] = LLVMInt32Type();
  }
  LLVMTypeRef func_type = LLVMFunctionType(ret_type, param_types, paramCount, 0);
  LLVMValueRef func = LLVMAddFunction(module, name, func_type);

  LLVMBasicBlockRef entry = LLVMAppendBasicBlock(func, "");
  LLVMPositionBuilderAtEnd(builder, entry);
  currentFunc = func;
  for (int i = 0; i < node->content.functionNode->bodyCount; i++) {
    codegenNext(node->content.functionNode->body[i]);
    if (i == node->content.functionNode->bodyCount - 2) isLastStatement = 1;
  }
  currentFunc = NULL;

  return func;
}

LLVMValueRef codegenNumber(Node *node) {
  return LLVMConstInt(LLVMInt32Type(), node->content.numberNode->value, 0);
}

LLVMValueRef codegenIdentifier(Node *node) {
  Variable *var = recallFromVariableCache(node->content.identifierNode->name);
  if (var != NULL) {
    return LLVMBuildLoad(builder, var->value, "");
  } else {
    printf("Call to undefined variable: %s.\n", node->content.identifierNode->name);
    abort();
  }
}

LLVMValueRef codegenBinop(Node *node) {
  BinopType type = node->content.binopNode->type;
  if (type == BINOP_BNOT || type == BINOP_NOT || type == BINOP_ASGN) {
    if (type == BINOP_BNOT) {
      LLVMValueRef RHS = codegenNext(node->content.binopNode->RHS);
      return LLVMBuildNot(builder, RHS, "");
    } else if (type == BINOP_ASGN) {
      if (node->content.binopNode->RHS->type == NODE_FUNCTION) {
        LLVMValueRef func = codegenFunction(node->content.binopNode->RHS, node->content.binopNode->LHS->content.identifierNode->name);
        // addToVariableCache(VAR_FUNCTION, func, node->content.binopNode->LHS->content.identifierNode->name);
        return NULL;
      } else {
        LLVMValueRef RHS = codegenNext(node->content.binopNode->RHS);
        LLVMValueRef alloca = LLVMBuildMalloc(builder, LLVMInt32Type(), "");
        addToVariableCache(VAR_VALUE, alloca, node->content.binopNode->LHS->content.identifierNode->name);
        LLVMBuildStore(builder, RHS, alloca);
        return RHS;
      }
    } else {
      LLVMValueRef RHS = codegenNext(node->content.binopNode->RHS);
      return LLVMBuildICmp(builder, LLVMIntEQ, RHS, LLVMConstInt(LLVMInt32Type(), 0, 0), "");
    }
  } else {
    LLVMValueRef LHS = codegenNext(node->content.binopNode->LHS);
    LLVMValueRef RHS = codegenNext(node->content.binopNode->RHS);
    switch (node->content.binopNode->type) {
      case BINOP_ADD:
        return LLVMBuildAdd(builder, LHS, RHS, "");
      case BINOP_SUB:
        return LLVMBuildSub(builder, LHS, RHS, "");
      case BINOP_MUL:
        return LLVMBuildMul(builder, LHS, RHS, "");
      case BINOP_DIV:
        return LLVMBuildUDiv(builder, LHS, RHS, "");
      case BINOP_BAND:
        return LLVMBuildAnd(builder, LHS, RHS, "");
      case BINOP_BOR:
        return LLVMBuildAnd(builder, LHS, RHS, "");
      case BINOP_BSR:
        return LLVMBuildLShr(builder, LHS, RHS, "");
      case BINOP_BSL:
        return LLVMBuildShl(builder, LHS, RHS, "");
      case BINOP_LT:
        return LLVMBuildIntCast2(builder, LLVMBuildICmp(builder, LLVMIntULT, LHS, RHS, ""), LLVMInt32Type(), 0, "");
      case BINOP_LTE:
        return LLVMBuildIntCast2(builder, LLVMBuildICmp(builder, LLVMIntULE, LHS, RHS, ""), LLVMInt32Type(), 0, "");
      case BINOP_GT:
        return LLVMBuildIntCast2(builder, LLVMBuildICmp(builder, LLVMIntUGT, LHS, RHS, ""), LLVMInt32Type(), 0, "");
      case BINOP_GTE:
        return LLVMBuildIntCast2(builder, LLVMBuildICmp(builder, LLVMIntUGE, LHS, RHS, ""), LLVMInt32Type(), 0, "");
      case BINOP_EQ:
        return LLVMBuildIntCast2(builder, LLVMBuildICmp(builder, LLVMIntEQ, LHS, RHS, ""), LLVMInt32Type(), 0, "");
      case BINOP_NEQ:
        return LLVMBuildIntCast2(builder, LLVMBuildICmp(builder, LLVMIntNE, LHS, RHS, ""), LLVMInt32Type(), 0, "");
      default: return NULL;
    }
  }
}

LLVMValueRef codegenFunctionCall(Node *node) {
  // TODO: Assert that callee is an identifier
  if (strncmp(node->content.functionCallNode->function->content.identifierNode->name, "return", 6) == 0) {
    if (node->content.functionCallNode->argsCount == 0) {
      return LLVMBuildRetVoid(builder);
    } else {
      return LLVMBuildRet(builder, codegenNext(node->content.functionCallNode->args[0]));
    }
  } else if (strncmp(node->content.functionCallNode->function->content.identifierNode->name, "if", 2) == 0) {
    Node *cond = node->content.functionCallNode->args[0];
    LLVMValueRef llvmCond = LLVMBuildICmp(builder, LLVMIntNE, codegenNext(cond), LLVMConstInt(LLVMInt32Type(), 0, 0), "");
    if (node->content.functionCallNode->argsCount == 2) {
      LLVMBasicBlockRef thenBlock = LLVMAppendBasicBlock(currentFunc, "");
      LLVMBasicBlockRef endBlock = LLVMAppendBasicBlock(currentFunc, "");
      LLVMPositionBuilderAtEnd(builder, LLVMGetEntryBasicBlock(currentFunc));
      LLVMBuildCondBr(builder, llvmCond, thenBlock, endBlock);
      LLVMPositionBuilderAtEnd(builder, thenBlock);
      if (node->content.functionCallNode->args[1]->type == NODE_FUNCTION) {
        for (int i = 0; i < node->content.functionCallNode->args[1]->content.functionNode->bodyCount; i++)
          codegenNext(node->content.functionCallNode->args[1]->content.functionNode->body[i]);
      } else {
        puts("Second argument to if() must be a function.");
        abort();
      }
      LLVMPositionBuilderAtEnd(builder, endBlock);
    } else if (node->content.functionCallNode->argsCount == 3) {
      if (isLastStatement) {
        LLVMBasicBlockRef thenBlock = LLVMAppendBasicBlock(currentFunc, "");
        LLVMBasicBlockRef elseBlock = LLVMAppendBasicBlock(currentFunc, "");
        LLVMPositionBuilderAtEnd(builder, LLVMGetEntryBasicBlock(currentFunc));
        LLVMBuildCondBr(builder, llvmCond, thenBlock, elseBlock);
        LLVMPositionBuilderAtEnd(builder, thenBlock);
        if (node->content.functionCallNode->args[1]->type == NODE_FUNCTION) {
          for (int i = 0; i < node->content.functionCallNode->args[1]->content.functionNode->bodyCount; i++)
            codegenNext(node->content.functionCallNode->args[1]->content.functionNode->body[i]);
        } else {
          puts("Second argument to if() must be a function.");
          abort();
        }
        LLVMPositionBuilderAtEnd(builder, elseBlock);
        if (node->content.functionCallNode->args[2]->type == NODE_FUNCTION) {
          for (int i = 0; i < node->content.functionCallNode->args[2]->content.functionNode->bodyCount; i++)
            codegenNext(node->content.functionCallNode->args[2]->content.functionNode->body[i]);
        } else {
          puts("Third argument to if() must be a function.");
          abort();
        }
      } else {
        LLVMBasicBlockRef thenBlock = LLVMAppendBasicBlock(currentFunc, "");
        LLVMBasicBlockRef elseBlock = LLVMAppendBasicBlock(currentFunc, "");
        LLVMBasicBlockRef endBlock = LLVMAppendBasicBlock(currentFunc, "");
        LLVMPositionBuilderAtEnd(builder, LLVMGetEntryBasicBlock(currentFunc));
        LLVMBuildCondBr(builder, llvmCond, thenBlock, elseBlock);
        LLVMPositionBuilderAtEnd(builder, thenBlock);
        if (node->content.functionCallNode->args[1]->type == NODE_FUNCTION) {
          int i = 0;
          for ( ; i < node->content.functionCallNode->args[1]->content.functionNode->bodyCount; i++)
            codegenNext(node->content.functionCallNode->args[1]->content.functionNode->body[i]);
          if (!(
            node->content.functionCallNode->args[1]->content.functionNode->body[i - 1]->type == NODE_FUNCTION
            && strncmp(node->content.functionCallNode->args[1]->content.functionNode->body[i - 1]->content.functionCallNode->function->content.identifierNode->name, "return", 6) == 0
          ))
            LLVMBuildBr(builder, endBlock);
        } else {
          puts("Second argument to if() must be a function.");
          abort();
        }
        LLVMPositionBuilderAtEnd(builder, elseBlock);
        if (node->content.functionCallNode->args[2]->type == NODE_FUNCTION) {
          int i = 0;
          for ( ; i < node->content.functionCallNode->args[2]->content.functionNode->bodyCount; i++)
            codegenNext(node->content.functionCallNode->args[2]->content.functionNode->body[i]);
          if (!(
            node->content.functionCallNode->args[1]->content.functionNode->body[i - 1]->type == NODE_FUNCTION
            && strncmp(node->content.functionCallNode->args[1]->content.functionNode->body[i - 1]->content.functionCallNode->function->content.identifierNode->name, "return", 6) == 0
          ))
            LLVMBuildBr(builder, endBlock);
        } else {
          puts("Third argument to if() must be a function.");
          abort();
        }
        LLVMPositionBuilderAtEnd(builder, endBlock);
      }
    } else {
      puts("Invalid number of arguments for if().");
      abort();
    }
    return NULL;
  } else {
    size_t argsSize = node->content.functionCallNode->argsCount;
    LLVMValueRef *args = malloc(argsSize * sizeof(LLVMValueRef));
    for (int i = 0; i < argsSize; i++) {
      args[i] = codegenNext(node->content.functionCallNode->args[i]);
    }

    Variable *var = recallFromVariableCache(node->content.functionCallNode->function->content.identifierNode->name);
    if (var != NULL) {
      return LLVMBuildCall(builder, var->value, args, argsSize, "");
    } else {
      abort();
    }
  }
}

// codegen stuff