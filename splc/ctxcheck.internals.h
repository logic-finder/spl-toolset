#ifndef CTXCHECK_INTERNALS_H
#define CTXCHECK_INTERNALS_H

#include <stdbool.h>

#include "msg.h"
#include "global.h"
#include "wrappers.h"
#include "array.adt.h"

typedef struct {
   const char *sourcefile;
   config_t *cfg;
   const char *reason;
   array_t *ls;
   array_t *errs;
   tree_t *nrtv;
   tree_t *act, *scene;  /* for tree traversal */
   tree_t *t;  /* temporary tree */
   node_t *n;  /* temporary tree.dat */
} ctxcheck_ctx_t;

typedef void ctxchecker_t(ctxcheck_ctx_t *octx);

static inline ctxchecker_t check_goto_unexistent;
static tree_callback_t check_goto_unexistent_route;
static inline ctxchecker_t check_goto_unexistent_act;
static inline ctxchecker_t check_goto_unexistent_scene;
static ctxchecker_t check_goto_unexistent_goto;

static void check_empty_scene(ctxcheck_ctx_t *octx);

static void regerr(ctxcheck_ctx_t *octx);

static void print_err(ctxcheck_ctx_t *octx);
static void trace(ctxcheck_ctx_t *octx, tree_t *t, const char *type);
static void semerr_unknown_label(ctxcheck_ctx_t *octx, const char *s);

#endif
