#ifndef TREE_ADT_TYPE_H
#define TREE_ADT_TYPE_H

#include <stddef.h>
#include <string.h>
#include "fatal.h"
#include "global.h"
#include "wrapper.h"
#include "tree.adt.h"

#define INIT_CMAX 2

struct tree {
   tree_t *parent;
   tree_t **children;
   int cmax;
   int clen;
   void *dat;
   size_t siz;
};

static void tree_prune_cb(tree_t *t);
static bool tree_full(tree_t *t);
static void tree_enlarge(tree_t *t);

#endif
