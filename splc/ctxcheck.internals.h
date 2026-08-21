#ifndef CTXCHECK_INTERNALS_H
#define CTXCHECK_INTERNALS_H

#include <stdbool.h>

#include "msg.h"
#include "global.h"
#include "wrappers.h"

typedef struct {
   const char *reason;
   tree_t *nrtv, *act, *scene;
   tree_t *t;  /* temporary tree */
   node_t *n;  /* temporary tree.dat */
   array_t *ls;
   optval_t *ov;
} ctxcheck_ctx_t;

typedef void ctxchecker_t(ctxcheck_ctx_t *octx);

static tree_callback_t ctxcheck_router;
static inline ctxchecker_t ctxcheck_act;
static inline ctxchecker_t ctxcheck_scene;
static ctxchecker_t ctxcheck_goto;

static void print_err(ctxcheck_ctx_t *octx);
static void trace(ctxcheck_ctx_t *octx, tree_t *t, const char *type);
static void semerr_unknown_label(ctxcheck_ctx_t *octx, const char *s);

#endif
