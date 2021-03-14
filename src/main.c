#include <roblang/parser.h>
#include <roblang/objgen.h>
#include <roblang/codegen.h>
#include <roblang/program.h>
#include <roblang/file.h>

extern Program program;

int main(int argc, char **argv) {
  if (argc < 2) return 1;

  setScript(readFile(argv[1]));

  codegenBinop(parseNextExpression(";"));

  outputObjectFile();

  return 0;
}
