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
  return !success; \

#endif