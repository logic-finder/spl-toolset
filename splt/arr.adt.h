#ifndef ARR_ADT_H
#define ARR_ADT_H

#include <stddef.h>

typedef struct arr arr_t;
typedef void arr_iterator_t(void *item, int idx);

arr_t *arr_create(void);
void arr_destroy(arr_t *arr);
void arr_append(arr_t *arr, const void *item, size_t siz);
void *arr_peek(arr_t *arr, int idx);
void arr_foreach(arr_t *arr, arr_iterator_t *iterate);
int arr_size(arr_t *arr);

#endif
