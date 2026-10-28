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

extern array_t *array_create(
   array_destructor_t *destruct,
   arena_t *arena
) {
   array_t *a;

   if (!(a = arena_malloc_fallback(arena, sizeof *a))) {
      return NULL;  /* nothing to cleanup yet; early exit */
   }

   a->container = arena_malloc_fallback(
      arena,
      INIT_MAX * ESIZ(a->container)
   );
   if (a->container == NULL) {
      goto cleanup;
   }

   a->len = 0;
   a->max = INIT_MAX;
   a->destruct = destruct;
   a->arena = arena;
   return a;

cleanup:
   free(a);
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

extern bool array_append(array_t *a, const void *item, size_t siz) {
   void *p;

   if (array_full(a) && !array_enlarge(a)) {
      return false;
   }

   if ((p = arena_malloc_fallback(a->arena, siz)) == NULL) {
      return false;
   }
   memcpy(p, item, siz);
   a->container[a->len++] = p;

   return true;
}

extern void *array_peek(array_t *a, int idx) {
   return a->container[idx];
}

extern size_t array_foreach(array_t *a, array_iterator_t *iterate, void *ctx) {
   size_t i;

   for (i = 0; i < a->len; i++) {
      if (!(*iterate)(a->container[i], i, ctx)) {
         return i;
      }
   }

   return i;
}

extern size_t array_size(array_t *a) {
   return a->len;
}

extern void array_sort(array_t *a, array_sorter_t *compare) {
   qsort(a->container, a->len, ESIZ(a->container), compare);
}

static inline bool array_full(array_t *a) {
   return (a->len == a->max);
}

static bool array_enlarge(array_t *a) {
   a->container = arena_realloc2x_arr_fallback(
      a->arena,
      a->container,
      &a->max,
      ESIZ(a->container)
   );
   return a->container
      ? true
      : false;
}
