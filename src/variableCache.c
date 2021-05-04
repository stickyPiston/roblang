#include <roblang/variableCache.h>

#include <stdlib.h>
#include <string.h>

static VariableCache cache = { NULL, 0 };

void addToVariableCache(VariableType t, LLVMValueRef v, char *l) {
  if (cache.count == 0) {
    cache.vars = malloc(sizeof(Variable));
    cache.vars[cache.count++] = (Variable){ t, v, l };
  } else {
    cache.vars = realloc(cache.vars, ++cache.count * sizeof(Variable));
    cache.vars[cache.count - 1] = (Variable){ t, v, l };
  }
}

Variable *recallFromVariableCache(char *l) {
  for (int i = 0; i < cache.count; i++) {
    if (strcmp(l, cache.vars[i].label) == 0) {
      return cache.vars + i;
    }
  }
  return NULL;
}