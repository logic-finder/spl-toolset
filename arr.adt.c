#include "arr.adt.h"
#include "arr.adt.type.h"

extern arr_t *arr_create(void) {
   arr_t *arr;

   arr = smalloc(sizeof *arr);
   arr->arr = smalloc(INIT_MAX * ESIZ(arr->arr));
   arr->len = 0;
   arr->max = INIT_MAX;

   return arr;
}

extern void arr_destroy(arr_t *arr) {
   for (int i = 0; i < arr->len; i++)
      free(arr->arr[i]);
   free(arr);
}

extern void arr_append(arr_t *arr, const void *item, size_t siz) {
   void *p;

   if (arr_full(arr)) arr_enlarge(arr);

   p = smalloc(siz);
   memcpy(p, item, siz);
   arr->arr[arr->len++] = p;
}

extern void *arr_peek(arr_t *arr, int idx) {
   return arr->arr[idx];
}

extern void arr_foreach(arr_t *arr, arr_iterator_t *iterate) {
   for (int i = 0; i < arr->len; i++)
      (*iterate)(arr->arr[i], i);
}

extern int arr_size(arr_t *arr) {
   return arr->len;
}

static inline bool arr_full(arr_t *arr) {
   return arr->len == arr->max;
}

static void arr_enlarge(arr_t *arr) {
   arr->max *= 2;
   arr->arr = srealloc_arr(
      arr->arr,
      arr->max,
      ESIZ(arr->arr)
   );
}

void arr_sort(arr_t *arr, arr_sorter_t *compare) {
   qsort(arr->arr, arr->len, ESIZ(arr->arr), compare);
}
