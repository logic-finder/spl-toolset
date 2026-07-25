#ifndef HASHTABLE_ADT_H
#define HASHTABLE_ADT_H

#include <stddef.h>
#include <stdbool.h>

typedef struct hashtable hashtable_t;

hashtable_t *ht_create(void);
void ht_destroy(hashtable_t *ht);
void ht_set(
   hashtable_t *ht,
   const char *key,
   void *val,
   size_t siz
);
void *ht_get(hashtable_t *ht, const char *key);
int ht_size(hashtable_t *ht);
bool ht_full(hashtable_t *ht);

#endif
