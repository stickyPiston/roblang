#ifndef ROBLANG_MODULE_H
#define ROBLANG_MODULE_H

#include <math.h>

typedef struct {
  unsigned char *bytes;
  int size;
} Program;

#define convertIntoBytes(value, endianess) \
  unsigned char bytes[sizeof(value)]; \
  do { \
    for (int z = 0; z < sizeof(value); z++) { \
      if (endianess) { \
        int devidant = lround(pow(2, (sizeof(value) - 1) * 8) - 1); \
        bytes[z] = (unsigned char)( ( (value << z * 8) & (0xff << (sizeof(value) - 1) * 8) ) / (devidant == 0 ? 1 : devidant) & 0xff ); \
      } else { \
        bytes[z] = (unsigned char)((value >> z * 8) & 0xff); \
      } \
    } \
  } while(0)

#define appendToProgram(program, value, endianess) \
  do { \
    if (program.size == 0) { \
      program.bytes = malloc(sizeof(value)); \
      convertIntoBytes(value, endianess); \
      for (int a = 0; a < sizeof(value); a++) \
        program.bytes[a] = bytes[a]; \
      program.size += sizeof(value); \
    } else { \
      program.bytes = realloc(program.bytes, program.size + sizeof(value)); \
      convertIntoBytes(value, endianess); \
      for (int a = 0; a < sizeof(value); a++) \
        program.bytes[program.size + a] = bytes[a]; \
      program.size += sizeof(value); \
    } \
  } while (0)

#endif
