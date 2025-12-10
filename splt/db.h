#ifndef DB_H
#define DB_H

#include <stdbool.h>

typedef bool querier_t(
   const char * restrict s,
   int * restrict ret
);

querier_t query_adj;
querier_t query_noun;

#endif
