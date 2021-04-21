#ifndef ROBLANG_SYMTABLE_H
#define ROBLANG_SYMTABLE_H

#include <mach-o/nlist.h>

typedef enum {
  SYMBOL_LOCAL,
  SYMBOL_EXTERN,
  SYMBOL_UNDEFINED
} SymbolType;

typedef struct {
  char *name;
  int address;
  int stringTableIndex;
  SymbolType type;
} Symbol;

typedef struct {
  Symbol *symbols;
  int size;
} SymTable;

typedef struct {
  char *string;
  int size;
} StringTable;

int addEntryToSymTable(char *name, int address, SymbolType type);
int findInSymTable(char *name);
void addToStringTable(Symbol *symbol);

#endif
