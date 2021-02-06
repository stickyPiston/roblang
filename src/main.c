#include <stdio.h>

#include <roblang/lexer.h>

// test code (will be removed)
int main(int argc, char **argv) {
  if (argc < 2) return 1;
  Token **tokens = NULL;
  lex(argv[1], &tokens);
  printf("%s\n", "This is the roblang main executable");
  printf("The output from the lexer is %s, type: %d\n", tokens[0]->value, tokens[0]->type);
  return 0;
}
