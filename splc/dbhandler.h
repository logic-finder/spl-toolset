#ifndef DBHANDLER_H
#define DBHANDLER_H

#include <stdbool.h>

typedef bool querier_rec_A(const char *key);
typedef bool querier_rec_B(const char *key, int *ret);

void dbload(void);
void dbunload(void);

querier_rec_A query_name;
querier_rec_A query_adj;
querier_rec_B query_noun;
querier_rec_B query_comp;

querier_rec_A query_adj_lower;
querier_rec_B query_comp_lower;

#endif
