#ifndef IRGEN_INTERNALS_H
#define IRGEN_INTERNALS_H

#include "global.h"
#include "wrappers.h"
#include "tree.adt.h"

static tree_t *plant_tree(
   const char *run,
   size_t len,
   irnodekind_t kind,
   int lnum,
   int lpos
);

static tree_t *graft_tree_s(
   tree_t *base,
   const char *run,
   size_t len,
   irnodekind_t kind,
   size_t lnum,
   size_t lpos
);

static tree_t *graft_tree_n(
   tree_t *base,
   int val,
   irnodekind_t kind,
   size_t lnum,
   size_t lpos
);

#endif
