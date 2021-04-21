#include <roblang/symtable.h>

#include <stdlib.h>
#include <string.h>

SymTable symTable = { NULL, 0 };
StringTable stringTable = { NULL, 0 };

int addEntryToSymTable(char *name, int address, SymbolType type) {
  char *symname = malloc(strlen(name) + 2);
  strcpy(symname, "_"); strcat(symname, name);

  if (symTable.size == 0) {
    symTable.symbols = malloc(sizeof(Symbol));
    Symbol symbol = (Symbol){ symname, address, 0, type };
    addToStringTable(&symbol);
    symTable.symbols[symTable.size++] = symbol;
  } else {
    symTable.symbols = realloc(symTable.symbols, ++symTable.size * sizeof(Symbol));
    Symbol symbol = (Symbol){ symname, address, 0, type };
    addToStringTable(&symbol);
    symTable.symbols[symTable.size - 1] = symbol;
  }
  return symTable.size - 1;
}

int findInSymTable(char *name) {
  for (int i = 0; i < symTable.size; i++) {
    if (strcmp(symTable.symbols[i].name, name) == 0) return i;
  }
  return -1;
}

void addToStringTable(Symbol *symbol) {
  if (stringTable.string == NULL) {
    int index = 0;
    stringTable.string = calloc(strlen(symbol->name) + 2 + 8 - (strlen(symbol->name) + 1) % 8, 1);
    symbol->stringTableIndex = stringTable.size;
    stringTable.size += strlen(symbol->name) + 2 + 8 - (strlen(symbol->name) + 1) % 8;
    stringTable.string[index++] = '\0';
    for (int j = 0; j < strlen(symbol->name); j++) {
      stringTable.string[index++] = symbol->name[j];
    }
    stringTable.string[index++] = '\0';
  } else {
    int index = stringTable.size;
    stringTable.string = realloc(stringTable.string, stringTable.size + strlen(symbol->name) + 2 + 8 - strlen(symbol->name) % 8);
    symbol->stringTableIndex = stringTable.size;
    memset(stringTable.string + stringTable.size, 0, strlen(symbol->name) + 2 + 8 - (strlen(symbol->name) + 1) % 8);
    stringTable.size += strlen(symbol->name) + 2 + 8 - (strlen(symbol->name) + 1) % 8;
    stringTable.string[index++] = '\0';
    for (int j = 0; j < strlen(symbol->name); j++) {
      stringTable.string[index++] = symbol->name[j];
    }
    stringTable.string[index++] = '\0';
  }
}
