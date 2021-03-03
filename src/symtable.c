#include <roblang/symtable.h>

#include <stdlib.h>
#include <string.h>

static SymTable table;

int addEntryToSymTable(char *name, int address) {
  char *symname = malloc(strlen(name) + 2);
  strcpy(symname, "_"); strcat(symname, name);
  if (table.size == 0) {
    table.symbols = malloc(sizeof(Symbol));
    table.symbols[table.size++] = (Symbol){ symname, address };
  } else {
    table.symbols = realloc(table.symbols, ++table.size * sizeof(Symbol));
    table.symbols[table.size - 1] = (Symbol){ symname, address };
  }
  return table.size - 1;
}

int getSymTableSize() {
  return table.size;
}

struct nlist_64 *getSymTable() {
  struct nlist_64 *abiTable = malloc(table.size * sizeof(struct nlist_64 *));
  for (int i = 0; i < table.size; i++) {
    struct nlist_64 symtabEntry = (struct nlist_64){
      .n_un.n_strx = 1,
      .n_type      = 0x0f,
      .n_sect      = 1,
      .n_desc      = 0x0,
      .n_value     = 0x0
    };
    abiTable[i] = symtabEntry;
  }
  return abiTable;
}

StringTable *getStringTable() {
  char *stringTable = NULL; int index = 0; int size = 0;
  for (int i = 0; i < table.size; i++) {
    if (stringTable == NULL) {
      stringTable = calloc(strlen(table.symbols[i].name) + 2 + 8 - (strlen(table.symbols[i].name) + 1) % 8, 1);
      size += strlen(table.symbols[i].name) + 2 + 8 - (strlen(table.symbols[i].name) + 1) % 8;
      stringTable[index++] = '\0';
      for (int j = 0; j < strlen(table.symbols[i].name); j++) {
        stringTable[index++] = table.symbols[i].name[j];
      }
      stringTable[index++] = '\0';
    } else {
      stringTable = realloc(stringTable, size + strlen(table.symbols[i].name) + 2 + 8 - (strlen(table.symbols[i].name) + 1) % 8);
      memset(stringTable + size, 0, strlen(table.symbols[i].name) + 2 + 8 - (strlen(table.symbols[i].name) + 1) % 8);
      size += strlen(table.symbols[i].name) + 2 + 8 - (strlen(table.symbols[i].name) + 1) % 8;
      stringTable[index++] = '\0';
      for (int j = 0; j < strlen(table.symbols[i].name); j++) {
        stringTable[index++] = table.symbols[i].name[j];
      }
      stringTable[index++] = '\0';
    }
  }

  StringTable *table = malloc(sizeof(*table));
  *table = (StringTable){ stringTable, size - 1 };
  return table;
}
