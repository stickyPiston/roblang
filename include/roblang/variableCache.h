#ifndef ROBLANG_VARIABLECACHE_H
#define ROBLANG_VARIABLECACHE_H

#include <llvm-c/Core.h>

typedef enum {
  VAR_VALUE,
  VAR_FUNCTION
} VariableType;

typedef struct {
  VariableType type;
  LLVMValueRef value;
  char *label;
} Variable;

typedef struct {
  Variable *vars;
  size_t count;
} VariableCache;

void addToVariableCache(VariableType, LLVMValueRef, char *);
Variable *recallFromVariableCache(char *);

#endif