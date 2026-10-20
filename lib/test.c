#include "test.h"
#include "test.internals.h"

extern void test_title(const char *filename) {
   safe_vprintf(
      "=== Running Unit Test for "
      Cbwhite "test/%s" Creset
      " ===\n",
      filename
   );
}

extern void test_result(size_t nfail) {
   safe_vprintf("UNIT TEST RESULT: %s\n" Creset,
      nfail == 0 ? Cbgreen "OK" : Cbred "FAILED"
   );
}
