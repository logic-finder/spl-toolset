#include "array.adt.h"
#include "array.adt.internals.h"

/*
 * TODO: write a new data structure, vector, which takes only one type
 * (1) do not use void **
 * (2) allocate memory contiguously
 * (3) minimize realloc calls
 * (4) reduce dereferene level
 * (5) use 2x+1 resize strategy (begins at 0)
 * (6) store the element size when init and use that size when alloc
 * Once done, replace array to vector if appropriate
 */

[[nodiscard]]
extern array_t *array_create(
   array_destructor_t *destruct,
   arena_t *arena
) {
   array_t *a = NULL;

   a = arena_malloc_fallback(arena, sizeof *a);
   if (a == NULL)
      goto OOM1;

   a->container = arena_malloc_fallback(
      arena,
      INIT_MAX * ESIZ(a->container)
   );
   if (a->container == NULL)
      goto OOM2;

   a->len = 0;
   a->max = INIT_MAX;
   a->destruct = destruct;
   a->arena = arena;

   return a;

OOM2:
   free(a);
OOM1:
   return NULL;
}

extern void array_destroy(array_t *a) {
   if (a == NULL) {
      return;
   }

   for (size_t i = 0; i < a->len; i++) {
      if (a->destruct != NULL) {
         a->destruct(a->container[i], i);
      }
      free(a->container[i]);
   }
   free(a->container);
   free(a);
}

[[nodiscard]]
extern bool array_append(array_t *a, const void *item, size_t siz) {
   void *p;

   if (array_full(a) && !array_enlarge(a)) {
      return false;
   }

   if ((p = arena_malloc(a->arena, siz)) == NULL) {
      return false;
   }
   memcpy(p, item, siz);
   a->container[a->len++] = p;

   return true;
}

extern void *array_peek(array_t *a, int idx) {
   return a->container[idx];
}

extern void array_foreach(array_t *a, array_iterator_t *iterate, void *ctx) {
   for (size_t i = 0; i < a->len; i++) {
      (*iterate)(a->container[i], i, ctx);
   }
}

extern size_t array_size(array_t *a) {
   return a->len;
}

extern void array_sort(array_t *a, array_sorter_t *compare) {
   qsort(a->container, a->len, ESIZ(a->container), compare);
}

static inline bool array_full(array_t *a) {
   return a->len == a->max;
}

static bool array_enlarge(array_t *a) {
   a->container = realloc2xarr(
      a->container,
      &a->max,
      ESIZ(a->container)
   );
   return a->container
      ? true
      : false;
}
