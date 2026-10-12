#ifndef SPLC_H
#define SPLC_H

#include "config.h"
#include "tree.adt.h"
#include "array.adt.h"

typedef struct {
   config_t *cfg;
   void **sects;
   array_t *ls;
   size_t lc, wc;
   array_t *toks;
   tree_t *pt;
   tree_t *irt;
   const char *sourcefile;
} compile_ctx_t;

#endif
