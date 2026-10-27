#ifndef TEST_H
#define TEST_H

#include <stddef.h>
#include <stdbool.h>

#define DIE() \
   do { \
      (void) printf( \
         "%s%s%s: assertion failed [%s:%zu]\n", \
         Cbcyan, __func__, Creset, __FILE__, __LINE__ \
      ); \
      flag = false; \
   } while (0)

void test_title(const char *filename);

void test_result(size_t nfail);

#endif
