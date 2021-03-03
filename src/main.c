#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <roblang/parser.h>
#include <roblang/objgen.h>
#include <roblang/codegen.h>
#include <roblang/program.h>

extern Program program;

int main(int argc, char **argv) {
  if (argc < 2) return 1;

  setScript(argv[1]);

  codegenBinop(parseNextExpression(";"));

  /* for (int i = 0; i < program.size; i++) { */
  /*   printf("%#x ", (unsigned char)*(program.bytes + i)); */
  /* } */
  outputObjectFile();

  return 0;
}
