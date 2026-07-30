#ifndef IROPTIMIZE_INTERNALS_H
#define IROPTIMIZE_INTERNALS_H

#include <limits.h>

#include "irgen.h"
#include "global.h"
#include "wrappers.h"

static void fold_const(tree_t *irt);
static void fold_const_work(tree_t *block, tree_t *new_block);
static void warn(irnode_t *n);

#endif
