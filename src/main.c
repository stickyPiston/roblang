#include <stdio.h>

#include <roblang/lexer.h>

// test code (will be removed)
int main() {
  Token **tokens = NULL;
  printf("%s\n", "This is the roblang main executable");
  printf("The output from the lexer is %d\n", lex("Hello", &tokens));
  return 0;
}
