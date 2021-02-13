#include <stdio.h>

#include <roblang/lexer.h>

// test code (will be removed)
int main(int argc, char **argv) {
  if (argc < 2) return 1;

  setScript(argv[1]);
  Token *token = NULL;
  while (1) {
    token = lexNextToken();
    if (token == NULL) break;
    printf("%s\n", token->value);
  }

  return 0;
}
