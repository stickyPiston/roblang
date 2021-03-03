#ifndef ROBLANG_STACK_H
#define ROBLANG_STACK_H

#include <inttypes.h>

typedef struct {
  char *label;
  uint16_t offset;
} Variable;

typedef struct {
  Variable *items;
  int size;
} Stack;

uint16_t stack_allocate(char *);
Variable *findInStack(char *);

#endif
