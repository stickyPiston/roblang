#include <roblang/stack.h>

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

static Stack stack = { NULL, 0 };

uint8_t stack_allocate(char *label) {
  // get the last stack item and retreive the index
  if (stack.size == 0) {
    uint8_t offset = 0xfc;
    stack.items = malloc(sizeof(Variable));
    stack.items[stack.size++] = (Variable){ label, offset };
    return offset;
  } else {
    uint8_t lastOffset = stack.items[stack.size - 1].offset;
    uint8_t newOffset = lastOffset - 4;
    stack.items = realloc(stack.items, ++stack.size * sizeof(Variable));
    stack.items[stack.size - 1] = (Variable){ label, newOffset };
    return newOffset;
  }
}

void stack_deallocate() {
  stack.items = realloc(stack.items, --stack.size * sizeof(Variable));
}

Variable *findInStack(char *label) {
  for (int i = 0; i < stack.size; i++) {
    if (strcmp(stack.items[i].label, label) == 0) return &stack.items[i];
  }
  return NULL;
}
