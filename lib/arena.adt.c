#include "arena.adt.h"
#include "arena.adt.internals.h"

/*================*
 | DEFINED INLINE |
 *================*/
[[nodiscard]]
extern void *arena_malloc_fallback(arena_t *arena, size_t siz);

[[nodiscard]]
extern void *arena_calloc_fallback(arena_t *arena, size_t nmemb, size_t siz);

[[nodiscard]]
extern void *arena_realloc2x(arena_t *arena, void *ptr, size_t *siz);

[[nodiscard]]
extern void *arena_realloc2x_arr_fallback(
   arena_t *arena,
   void *ptr,
   size_t *cnt,
   size_t esiz
);

extern void arena_free_fallback(arena_t *arena, void *ptr);

/*======================*
 | EXTERNAL DEFINITIONS |
 *======================*/
[[nodiscard]]
extern arena_t *arena_create(void) {
   arena_t *arena;

   if (!(arena = malloc(sizeof *arena))) {
      goto err1;
   }
   if (!(arena->regions = list_create(destruct_region))) {
      goto err2;
   }
   arena->current = NULL;
   arena->align = sizeof (void *);  /* assumes not 0 */

   return arena;

err2:
   free(arena);
err1:
   return NULL;
}

[[nodiscard]]
extern bool arena_info(const char *execname) {
   int nchar = printf(
      "%s: default region size set " STR(ARENA_SIZ_DEFAULT) "\n",
      execname
   );
   return (0 <= nchar) ? true : false;
}

extern void arena_cleanup(arena_t *arena) {
   list_cleanup(arena->regions);
   arena->current = NULL;
}

extern void arena_destroy(arena_t *arena) {
   if (!arena) {
      return;
   }
   list_destroy(arena->regions);
   free(arena);
}

[[nodiscard]]
extern void *arena_malloc(arena_t *arena, size_t siz) {
   if (siz == 0) {
      return NULL;
   }

   if (need_new_region(arena, siz) && !add_new_region(arena, siz)) {
      return NULL;
   }

   arena->current->last_alloc =
      (char *) arena->current->buf + arena->current->pos;
   arena->current->pos += siz;

   return arena->current->last_alloc;
}

[[nodiscard]]
extern void *arena_calloc(arena_t *arena, size_t nmemb, size_t siz) {
   char *ret;

   if (nmemb == 0 || siz == 0) {
      return NULL;
   }
   if (nmemb > SIZE_MAX / siz) {
      return NULL;
   }

   if (!(ret = arena_malloc(arena, nmemb * siz))) {
      return NULL;
   }

   for (size_t i = 0; i < nmemb * siz; i++) {
      ret[i] = 0;  /* sizeof char == 1 */
   }

   return ret;
}

[[nodiscard]]
extern void *arena_realloc2x_arr(
   arena_t *arena,
   void *ptr,
   size_t *nmemb,
   size_t esiz
) {
   size_t old_nmemb, new_nmemb;
   size_t old_siz, new_siz;
   void *ret;

   if (*nmemb == 0 || esiz == 0) {
      return NULL;
   }

   /* Doubles the nmemb */
   old_nmemb = *nmemb;
   if (old_nmemb > SIZE_MAX / 2 || esiz > SIZE_MAX / (old_nmemb * 2)) {
      return NULL;
   }
   new_nmemb = old_nmemb * 2;
   *nmemb = new_nmemb;

   old_siz = old_nmemb * esiz;
   new_siz = new_nmemb * esiz;

   if (ptr == NULL) {
      return arena_malloc(arena, new_siz);
   }

   if (!need_new_region(arena, new_siz)) {
      /* The fact that `need_new_region()` has returned false
      means the current region has enough space to hold
      the doubled size */

      /* in-place realloc? */
      if (arena->current->last_alloc == ptr) {
         arena->current->pos += old_siz;
         return arena->current->last_alloc;
      }
   }

   if (!(ret = arena_malloc(arena, new_siz))) {
      return NULL;
   }
   memcpy(ret, ptr, old_siz);
   return ret;
}

static void destruct_region(void *item, size_t idx) {
   arena_region_t *region;

   (void) idx;
   region = item;
   free(region->buf);
}

[[nodiscard]]
static bool need_new_region(arena_t *arena, size_t req_siz) {
   size_t rem;

   if (list_size(arena->regions) == 0) {
      return true;
   }

   /* 0 <= rem < align */
   rem = arena->current->pos % arena->align;
   if (rem != 0) {
      arena->current->pos -= rem;
      arena->current->pos += arena->align;
   }

   if (arena->current->cap <= arena->current->pos) {
      return true;
   }

   if (req_siz > arena->current->cap - arena->current->pos) {
      return true;
   }

   return false;
}

[[nodiscard]]
static bool add_new_region(arena_t *arena, size_t req_siz) {
   arena_region_t region = {0};

   if (!(region.buf = malloc(region.cap))) {
      return false;
   }
   region.cap = (req_siz > ARENA_SIZ_DEFAULT)
      ? req_siz
      : ARENA_SIZ_DEFAULT;
   region.last_alloc = NULL;

   arena->current = list_push(arena->regions, &region, sizeof region);

   return true;
}
