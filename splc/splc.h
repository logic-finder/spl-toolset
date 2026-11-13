#ifndef SPLC_H
#define SPLC_H

#include <stdbool.h>

#include "config.h"
#include "tree.adt.h"
#include "array.adt.h"
#include "arena.adt.h"
#include "list.adt.h"

#define PTRACE(ctx) do {            \
   list_push(                       \
      ctx->ptrace,                  \
      & (ptrace_info_t) {           \
            .funcname = __func__,   \
            .filename = __FILE__,   \
            .ln = __LINE__          \
      },                            \
      sizeof (ptrace_info_t)        \
   );                               \
} while (0)

#define PH_COPY(dest, src) do {  \
   dest->perr = src->perr;       \
   dest->phdlr = src->phdlr;     \
   dest->pnctx = src->pnctx;     \
} while (0)

#define PH_RESET(dest) do {   \
   dest->perr = NULL;         \
   dest->phdlr = NULL;        \
   dest->pnctx = NULL;        \
} while (0)

typedef struct {
   const char *funcname;
   const char *filename;
   size_t ln;
} ptrace_info_t;

typedef void panic_handler(compile_ctx_t *cctx);

typedef struct {
   config_t *cfg;
   array_t *ls;
   size_t lc, wc;
   array_t *toks;
   tree_t *pt;
   tree_t *irt;
   const char *sourcefile;
   arena_t *arena;
   list_t *ptrace;  // panic trace: list of ptrace_info_t
   const char *perr;  // panic error: fatal error reason
   panic_handler *phdlr;  // panic handler: fatal error handler
   void *pnctx;  // panic context: fatal error context
} compile_ctx_t;

#endif
