#include "common.h"

extern uint32_t endrev32(uint32_t v);

extern bool isle(void) {
   int v = 1;
   return ((char *) &v)[0];
}

extern size_t min(size_t a, size_t b) {
   return (a < b) ? a : b;
}

extern size_t min3(size_t a, size_t b, size_t c) {
   size_t t = a;
   if (b < t) t = b;
   if (c < t) t = c;
   return t;
}
