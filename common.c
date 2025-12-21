#include "common.h"

extern bool isle(void) {
   int v = 1;
   return ((char *) &v)[0];
}

extern uint32_t endrev32(uint32_t v);
