#ifndef ROBLANG_SYMTABLE_H
#define ROBLANG_SYMTABLE_H

#include <mach-o/nlist.h>

typedef struct {
  char *name;
  int address;
} Symbol;

typedef struct {
  Symbol *symbols;
  int size;
} SymTable;

typedef struct {
  char *string;
  int size;
} StringTable;

/**
 * Function that adds a symbol to the global symbol table
 * @returns The index of the newly added symbol
 */
int addEntryToSymTable(char *name, int address);

int getSymTableSize();

/**
 * @returns The symbol table in the format for the mach-o object files
 */
struct nlist_64 *getSymTable();

/**
 * @returns The string table in useable format for mach-o object files
 */
StringTable *getStringTable();

#endif