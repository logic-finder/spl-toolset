#ifndef TREE_ADT_INTERNALS_H
#define TREE_ADT_INTERNALS_H

#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include "fatal.h"
#include "common.h"
#include "wrappers.h"

#define INIT_CMAX 2

struct tree {
   tree_t *parent;
   tree_t **children;
   size_t cmax;
   size_t clen;
   void *dat;
   size_t siz;
};

static void prune_callback(tree_t *t);
static bool tree_full(const tree_t *t);
static void tree_enlarge(tree_t *t);

#endif
