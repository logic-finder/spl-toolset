#ifndef IRDUMP_H
#define IRDUMP_H

#include "splc.h"
#include "tree.adt.h"

void irdump(compile_ctx_t *cctx);
void debug_print_irnode(tree_t *t, int lv, void *ctx);

#endif
