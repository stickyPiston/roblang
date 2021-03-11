#ifndef ROBLANG_STACK_H
#define ROBLANG_STACK_H

#include <inttypes.h>

typedef uint8_t Offset;

typedef struct {
  char *label;
  Offset offset;
} Variable;

typedef struct {
  Variable *items;
  int size;
} Stack;

Offset    stack_allocate(char *);
Variable *findInStack(char *);
void      stack_deallocate();

#endif
