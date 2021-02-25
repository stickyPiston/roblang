#ifndef ROBTEST_MAIN_H
#define ROBTEST_MAIN_H

typedef struct {
  int (*func)();
  char *name;
} TestCase;

#define runTests(tests) \
  setvbuf(stdout, NULL, _IONBF, 0); \
  int size = sizeof(tests) / sizeof(tests[0]), success = 1; \
  for (int i = 0; i < size; i++) { \
    printf("Executing test %d: %s", i + 1, tests[i].name); \
    int result = tests[i].func(); \
    if (result == 1) { \
      fprintf(stdout, " -- %s\n", "SUCCESS"); \
      success = success && 1; \
    } else { \
      fprintf(stderr, " -- %s\n", "FAILURE"); \
      success = 0; \
    } \
  } \
  return !success;

#define generateRandomIdentifier(identifier, length) \
  identifier = calloc(length + 1, sizeof(char)); \
  char character = (rand() % 26) + 'A'; \
  identifier[0] = character; \
  for (int i = 1; i < length; i++) { \
    identifier[i] = rand() % 25 + 'a'; break; \
  }

#define generateRandomNumber(buffer) \
  uint32_t randomNum = rand() % 4294967296; \
  char buffer[12]; \
  snprintf(buffer, 11, "%d", randomNum); \

#define freeArray(array, size) \
  for (int i = 0; i < size; i++) { \
    free(array[i]); \
  } \
  free(array);

#endif