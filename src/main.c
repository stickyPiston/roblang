#include <stdio.h>

#include <roblang/lexer.h>
#include <roblang/parser.h>

// test code (will be removed)
int main(int argc, char **argv) {
  if (argc < 2) return 1;

  setScript(argv[1]);
  parseNextExpression(";");

  return 0;
}
