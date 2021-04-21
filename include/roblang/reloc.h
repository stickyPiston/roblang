#ifndef ROBLANG_RELOC_H
#define ROBLANG_RELOC_H

typedef struct {
  int address;
  int symbol;
} Relocation;

typedef struct {
  Relocation *relocs;
  int size;
} RelocationTable;

void insertIntoRelocTable(int address, int symbol);

#endif
