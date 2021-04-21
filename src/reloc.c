#include <roblang/reloc.h>

#include <stdlib.h>

RelocationTable relocTable = { NULL, 0 };

void insertIntoRelocTable(int address, int symbol) {
  if (relocTable.size == 0) {
    relocTable.relocs = malloc(sizeof(Relocation));
    relocTable.relocs[relocTable.size++] = (Relocation){ address, symbol };
  } else {
    relocTable.relocs = realloc(relocTable.relocs, ++relocTable.size * sizeof(Relocation));
    relocTable.relocs[relocTable.size - 1] = (Relocation){ address, symbol };
  }
}
