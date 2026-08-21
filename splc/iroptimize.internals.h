#ifndef IROPTIMIZE_INTERNALS_H
#define IROPTIMIZE_INTERNALS_H

#include <limits.h>

#include "irgen.h"
#include "global.h"
#include "wrappers.h"

typedef struct {
   tree_t *irt, *nrtv;
   array_t *ls;
   optval_t *ov;
} iroptmiz_ctx_t;

static void fold_const(iroptmiz_ctx_t *zctx);
static void fold_const_work(iroptmiz_ctx_t *zctx, tree_t *block, tree_t *new_block);
static void warn(iroptmiz_ctx_t *zctx, size_t lnum, size_t lpos);

#endif
