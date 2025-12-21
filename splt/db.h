#ifndef DB_H
#define DB_H

#include <stdbool.h>

typedef enum querykind {
   QUERYKIND_NAME,
   QUERYKIND_ADJ,
   QUERYKIND_NOUN,
   QUERYKIND_COMP
} querykind_t;

typedef bool querier_rec_A(const char *key);
typedef bool querier_rec_B(const char *key, int *ret);

void dbload(void);
void dbunload(void);

querier_rec_A query_name;
querier_rec_A query_adj;
querier_rec_B query_noun;
querier_rec_B query_comp;

#endif
