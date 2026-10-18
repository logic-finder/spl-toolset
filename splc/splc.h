#ifndef SPLC_H
#define SPLC_H

#include "config.h"
#include "tree.adt.h"
#include "array.adt.h"
#include "arena.adt.h"

typedef struct {
   config_t *cfg;
   array_t *ls;
   size_t lc, wc;
   array_t *toks;
   tree_t *pt;
   tree_t *irt;
   const char *sourcefile;
   arena_t *arena;
} compile_ctx_t;

#endif
