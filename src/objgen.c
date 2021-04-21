#include <stdio.h>
#include <stdlib.h>

#include <roblang/reloc.h>
#include <roblang/objgen.h>
#include <roblang/program.h>
#include <roblang/symtable.h>

#include <mach-o/reloc.h>
#include <mach-o/nlist.h>
#include <mach-o/loader.h>

extern Program program;
extern SymTable symTable;
extern RelocationTable relocTable;
extern StringTable stringTable;

void outputObjectFile() {
  const unsigned char *asmCode = program.bytes;
  const int asmSize = program.size;

  struct mach_header_64 *header = calloc(1, sizeof(*header));
  *header = (struct mach_header_64){
    .magic      = MH_MAGIC_64,
    .cputype    = 0x01000007,
    .cpusubtype = 0x03,
    .filetype   = 0x1,
    .ncmds      = 0x4,        // FIXME: make this variable
    .flags      = 0x0
  };

  // Sort the symbols into the correct order: local -> extern -> undefined
  struct nlist_64 *localsyms = malloc(symTable.size * sizeof(struct nlist_64));
  int nlocalsyms = 0;
  struct nlist_64 *externsyms = malloc(symTable.size * sizeof(struct nlist_64));
  int nexternsyms = 0;
  struct nlist_64 *undefsyms = malloc(symTable.size * sizeof(struct nlist_64));
  int nundefsyms = 0;
  for (int i = 0; i < symTable.size; i++) {
    struct nlist_64 symtabEntry = (struct nlist_64){
      .n_un.n_strx = symTable.symbols[i].stringTableIndex + 1,
      .n_type      = symTable.symbols[i].type == SYMBOL_EXTERN ? 0x0f : 0x01,
      .n_sect      = symTable.symbols[i].type == SYMBOL_EXTERN ? 1 : 0,
      .n_desc      = 0x0,
      .n_value     = symTable.symbols[i].address
    };
    switch (symTable.symbols[i].type) {
      case SYMBOL_LOCAL: { localsyms[nlocalsyms++] = symtabEntry; } break;
      case SYMBOL_EXTERN: { externsyms[nexternsyms++] = symtabEntry; } break;
      case SYMBOL_UNDEFINED: { undefsyms[nundefsyms++] = symtabEntry; } break;
    }
  }

  struct dysymtab_command *dysymtab = calloc(1, sizeof(*dysymtab));
  *dysymtab = (struct dysymtab_command){
    .cmd = LC_DYSYMTAB,
    .cmdsize = sizeof(*dysymtab),
    .ilocalsym = 0,
    .nlocalsym = nlocalsyms,
    .iextdefsym = nlocalsyms,
    .nextdefsym = nexternsyms,
    .iundefsym = nlocalsyms + nexternsyms,
    .nundefsym = nundefsyms
  };

  struct build_version_command *buildInfo = calloc(1, sizeof(*buildInfo));
  *buildInfo = (struct build_version_command){
    .cmd      = LC_BUILD_VERSION,
    .cmdsize  = sizeof(*buildInfo),
    .platform = PLATFORM_MACOS,
    .minos    = 0x0a0f06, // TODO: Make a function for this
    .sdk      = 0x0,
    .ntools   = 0
  };

  struct relocation_info *relocs = calloc(relocTable.size, sizeof(*relocs));
  for (int i = 0; i < relocTable.size; i++) {
    relocs[i] = (struct relocation_info){
      .r_address   = relocTable.relocs[i].address,
      .r_symbolnum = relocTable.relocs[i].symbol,
      .r_pcrel     = 1, // map 0b00101101 to the bit fields, but the bit fields are reversed in order.
      .r_length    = 2,
      .r_extern    = 1,
      .r_type      = 2
    };
  }

  struct segment_command_64 *segment = calloc(1, sizeof(*segment));
  *segment = (struct segment_command_64){
    .cmd       = LC_SEGMENT_64,
    .cmdsize   = sizeof(*segment) + sizeof(struct section_64),
    .segname   = 0x0,
    .vmaddr    = 0x0,
    .vmsize    = asmSize,
    .fileoff   = sizeof(*dysymtab) + sizeof(*header) + sizeof(*segment) + sizeof(struct section_64) + sizeof(*buildInfo) + sizeof(struct symtab_command),
    .filesize  = asmSize,
    .maxprot   = 0x7,     // Afaik, this can stay constant
    .initprot  = 0x7,     // Afaik, this can stay constant
    .nsects    = 0x1,     // I'm not sure if this needs to be variable...
    .flags     = 0x0
  };

  header->sizeofcmds = segment->fileoff - sizeof(*header);

  struct symtab_command *symtab = calloc(1, sizeof(*symtab));
  *symtab = (struct symtab_command){
    .cmd     = LC_SYMTAB,
    .cmdsize = sizeof(*symtab),
    .symoff  = sizeof(*dysymtab) + sizeof(*header) + sizeof(*segment) + sizeof(struct section_64) + sizeof(*buildInfo) + sizeof(*symtab) + relocTable.size * sizeof(*relocs) + asmSize,
    .nsyms   = symTable.size,
    .strsize = stringTable.size
  };
  symtab->stroff = symtab->symoff + symTable.size * sizeof(struct nlist_64);

  struct section_64 *section = calloc(1, sizeof(*section));
  *section = (struct section_64){
    .sectname = "__text",
    .segname  = "__TEXT",
    .addr     = 0x0,
    .size     = asmSize,
    .offset   = sizeof(*dysymtab) + sizeof(*header) + sizeof(*segment) + sizeof(*section) + sizeof(*buildInfo) + sizeof(*symtab),
    .align    = 0x0,
    .reloff   = sizeof(*dysymtab) + sizeof(*header) + sizeof(*segment) + sizeof(*section) + sizeof(*buildInfo) + sizeof(*symtab) + asmSize,
    .nreloc   = relocTable.size,
    .flags    = 0x80000400
  };

  const char *file = "out.o";
  FILE *objectFile = fopen(file, "w");
  if (objectFile == NULL) {
    fprintf(stderr, "Error opening %s", file);
    abort();
  }

  fwrite(header,              sizeof(*header),                             1,       objectFile);
  fwrite(segment,             sizeof(*segment),                            1,       objectFile);
  fwrite(section,             sizeof(*section),                            1,       objectFile);
  fwrite(buildInfo,           sizeof(*buildInfo),                          1,       objectFile);
  fwrite(symtab,              sizeof(*symtab),                             1,       objectFile);
  fwrite(dysymtab,            sizeof(*dysymtab),                           1,       objectFile);
  fwrite(asmCode,             sizeof(char),                                asmSize, objectFile);
  fwrite(relocs,              relocTable.size * sizeof(*relocs),           1,       objectFile);
  fwrite(localsyms,           nlocalsyms * sizeof(struct nlist_64),        1,       objectFile);
  fwrite(externsyms,          nexternsyms * sizeof(struct nlist_64),       1,       objectFile);
  fwrite(undefsyms,           nundefsyms * sizeof(struct nlist_64),        1,       objectFile);
  fwrite(stringTable.string,  stringTable.size,                            1,       objectFile);
}

