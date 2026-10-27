#ifndef ARRAY_ADT_INTERNALS_H
#define ARRAY_ADT_INTERNALS_H

#include <string.h>

#include "common.h"
#include "wrappers.h"

#define INIT_MAX 128

struct array {
   size_t len, max;
   void **container;
   array_destructor_t *destruct;
   arena_t *arena;
};

static inline bool array_full(array_t *a);

static bool array_enlarge(array_t *a);

#endif
