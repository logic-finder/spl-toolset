#ifndef HASHTABLE_ADT_INTERNALS_H
#define HASHTABLE_ADT_INTERNALS_H

#include <string.h>
#include "common.h"
#include "wrappers.h"

#define HASHTABLE_SIZ 1249  /* prime */

typedef enum bucketstate {
   BUCKETSTATE_EMPTY,
   BUCKETSTATE_FULL,
   BUCKETSTATE_DELETED
} bucketstate_t;

typedef struct bucket {
   void *dat;
   bucketstate_t state;
} bucket_t;

struct hashtable {
   int siz;
   bucket_t *arr;
};

static int hash(const char *s, unsigned int offset);

#endif
