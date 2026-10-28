#include "hashtable.adt.h"
#include "hashtable.adt.internals.h"

static int hash(const char *s, unsigned int offset) {
   unsigned long v = 5381;
   char c;

   while ((c = *s++)) {
      v = ((v << 5) + v) + c + offset;
   }

   return v % HASHTABLE_SIZ;
}

extern hashtable_t *ht_create(void) {
   hashtable_t *ht;

   if (!(ht = malloc(sizeof *ht))) {
      return NULL;
   }
   if (!(ht->arr = malloc(HASHTABLE_SIZ * ESIZ(ht->arr)))) {
      goto cleanup;
   }
   ht->siz = 0;

   for (int i = 0; i < HASHTABLE_SIZ; i++) {
      ht->arr[i].dat = NULL;
      ht->arr[i].state = BUCKETSTATE_EMPTY;
   }
   return ht;

cleanup:
   free(ht);
   return NULL;
}

extern void ht_destroy(hashtable_t *ht) {
   for (int i = 0; i < HASHTABLE_SIZ; i++) {
      free(ht->arr[i].dat);
   }
   free(ht->arr);
   free(ht);
}

extern bool ht_set(
   hashtable_t *ht,
   const char *key,
   void *val,
   size_t siz
) {
   int idx, offset;
   bucket_t *bucket;

   if (ht_full(ht)) {
      return false;
   }

   idx = hash(key, 0);
   for (offset = 0; /* n/a */; /* n/a */) {
      bucket = ht->arr + idx;
      if (bucket->state != BUCKETSTATE_FULL) {
         break;
      }
      idx = hash(key, ++offset);
   }

   bucket->state = BUCKETSTATE_FULL;
   if (!(bucket->dat = malloc(siz))) {
      return false;
   }
   memcpy(bucket->dat, val, siz);
   ht->siz++;

   return true;
}

[[nodiscard]]
extern void *ht_get(hashtable_t *ht, const char *key) {
   int idx, offset;
   bucket_t *bucket;

   idx = hash(key, 0);
   for (offset = 0; /* n/a */; /* n/a */) {
      bucket = ht->arr + idx;
      switch (bucket->state) {
         case BUCKETSTATE_EMPTY   : return NULL;
         case BUCKETSTATE_FULL    : return bucket->dat;
         case BUCKETSTATE_DELETED : break;
         default: ;  /* never happens */
      }
      idx = hash(key, ++offset);
   }

   return bucket->dat;
}

extern int ht_size(hashtable_t *ht) {
   return ht->siz;
}

extern bool ht_full(hashtable_t *ht) {
   return (ht->siz == HASHTABLE_SIZ);
}
