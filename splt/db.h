#ifndef DB_H
#define DB_H

#include <stdbool.h>

typedef enum querykind {
   QUERYKIND_ADJ,
   QUERYKIND_NOUN,
   QUERYKIND_COMP
} querykind_t;

bool query(
   querykind_t kind,
   const char * restrict s,
   int * restrict ret
);

#endif
