#ifndef SPLC_INTERNALS_H
#define SPLC_INTERNALS_H

#include "tree.adt.h"
#include "array.adt.h"

typedef struct optflg {
   unsigned int
      exe: 1,  /* --exe */
      imd: 1,  /* --keep-intermediate */
      dsc: 1,  /* --describe */
      kwi: 1,  /* --kawaii */
      lng: 1,  /* --lang */
      ret: 1,  /* --ret */
      dbg: 1,  /* -g */
      opt: 1,  /* -O */
      dmp: 1,  /* -S */
      eoo: 1,  /* -- */
      tgt: 1,  /* --target */
      drn: 1,  /* --dry-run */
      hlp: 1,  /* --help */
      vsn: 1;  /* --version */
} optflg_t;

typedef struct optval {
   const char *ret, *lng, *src;
} optval_t;

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
