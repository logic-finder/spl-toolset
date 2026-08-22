#ifndef SPLC_INTERNALS_H
#define SPLC_INTERNALS_H

#include "tree.adt.h"
#include "array.adt.h"

typedef struct optflg optflg_t;
typedef struct optval optval_t;

typedef struct {
   int argc;
   const char **argv;
   optflg_t *of;
   optval_t *ov;
   void **sects;
   array_t *ls;
   size_t lc, wc;
   array_t *toks;
   tree_t *pt;
   tree_t *irt;
} compile_ctx_t;

#endif
