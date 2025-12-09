#ifndef ARR_ADT_TYPES_H
#define ARR_ADT_TYPES_H

#include <string.h>
#include <stdbool.h>
#include "global.h"
#include "arr.adt.h"
#include "wrapper.h"

#define INIT_MAX 128

struct arr {
   int len;
   int max;
   void **arr;
};

static inline bool arr_full(arr_t *arr);
static void arr_enlarge(arr_t *arr);

#endif
