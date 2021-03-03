#include <stdio.h>
#include <stdlib.h>

#include <roblang/symtable.h>
#include <roblang/objgen.h>
#include <roblang/program.h>

#include <mach-o/nlist.h>
#include <mach-o/loader.h>

extern Program program;

void outputObjectFile() {
  const unsigned char *asmCode = program.bytes;
  const int asmSize = program.size;

  const char *file = "out.o";
  FILE *objectFile = fopen(file, "w");
  if (objectFile == NULL) {
    fprintf(stderr, "Error opening %s", file);
    abort();
  }

  struct section_64 *section = calloc(1, sizeof(*section));
  *section = (struct section_64){
    .sectname = "__text",
    .segname  = "__TEXT",
    .addr     = 0x0,
    .size     = asmSize,
    .offset   = 232,      // Same as .fileoff in the segment
    .align    = 0x0,
    .reloff   = 0x0,      // I'm not sure what relocations are, we'll see later
    .nreloc   = 0x0,
    .flags    = 0x80000400
  };

  struct segment_command_64 *segment = calloc(1, sizeof(*segment));
  *segment = (struct segment_command_64){
    .cmd       = LC_SEGMENT_64,
    .cmdsize   = sizeof(*segment) + sizeof(*section),
    .segname   = 0x0,
    .vmaddr    = 0x0,
    .vmsize    = asmSize,
    .fileoff   = 232,     // FIXME: make this variable
    .filesize  = asmSize,
    .maxprot   = 0x7,     // Afaik, this can stay constant
    .initprot  = 0x7,     // Afaik, this can stay constant
    .nsects    = 0x1,     // I'm not sure if this needs to be variable...
    .flags     = 0x0
  };

  struct build_version_command *buildInfo = calloc(1, sizeof(*buildInfo));
  *buildInfo = (struct build_version_command){
    .cmd      = LC_BUILD_VERSION,
    .cmdsize  = sizeof(*buildInfo),
    .platform = PLATFORM_MACOS,
    .minos    = 0x0a0f06,
    .sdk      = 0x0,
    .ntools   = 0
  };

  /* addEntryToSymTable("_main", 0); */
  StringTable *stringTable = getStringTable();

  struct symtab_command *symtab = calloc(1, sizeof(*symtab));
  *symtab = (struct symtab_command){
    .cmd     = LC_SYMTAB,
    .cmdsize = sizeof(*symtab),
    .symoff  = 232 + asmSize,   // TODO: make this variable
    .nsyms   = 1,
    .strsize = 8
  };
  symtab->stroff = symtab->symoff + getSymTableSize() * sizeof(struct nlist_64);

  struct mach_header_64 *header = calloc(1, sizeof(*header));
  *header = (struct mach_header_64){
    .magic      = MH_MAGIC_64,
    .cputype    = 0x01000007,
    .cpusubtype = 0x03,
    .filetype   = 0x1,
    .ncmds      = 0x3,        // FIXME: make this variable
    .sizeofcmds = 200,        // FIXME: make this variable (offset - header size)
    .flags      = 0x0
  };

  fwrite(header,              sizeof(*header),                             1,       objectFile);
  fwrite(segment,             sizeof(*segment),                            1,       objectFile);
  fwrite(section,             sizeof(*section),                            1,       objectFile);
  fwrite(buildInfo,           sizeof(*buildInfo),                          1,       objectFile);
  fwrite(symtab,              sizeof(*symtab),                             1,       objectFile);
  fwrite(asmCode,             sizeof(char),                                asmSize, objectFile);
  fwrite(getSymTable(),       getSymTableSize() * sizeof(struct nlist_64), 1,       objectFile);
  fwrite(stringTable->string, stringTable->size,                           1,       objectFile);
}
