#ifndef ARRAY_ADT_H
#define ARRAY_ADT_H

#include <stddef.h>

typedef struct array array_t;
typedef void array_iterator_t(void *item, int idx);
typedef int array_sorter_t(const void *d1, const void *d2);

array_t *array_create(void);
void array_destroy(array_t *a);
void array_append(array_t *a, const void *item, size_t siz);
void *array_peek(array_t *a, int idx);
void array_foreach(array_t *a, array_iterator_t *iterate);
int array_size(array_t *a);
void array_sort(array_t *a, array_sorter_t *compare);

#endif
