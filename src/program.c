#include <roblang/program.h>

int getCurrentProgramAddress(Program *p) {
  return p->size - 1;
}
