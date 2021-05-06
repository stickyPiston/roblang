#include <roblang/file.h>

#include <stdio.h>
#include <stdlib.h>

char *readFile(char *filename) {
  FILE *fp = fopen(filename, "r");
  fseek(fp, 0, SEEK_END);
  int fileSize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char *fileContents = calloc(fileSize + 1, 1);
  fread(fileContents, 1, fileSize, fp);

  fclose(fp);
  return fileContents;
}
