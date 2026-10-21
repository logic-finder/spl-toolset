#ifndef CTXCHECK_INTERNALS_H
#define CTXCHECK_INTERNALS_H

#include <stdbool.h>

#include "msg.h"
#include "global.h"
#include "wrappers.h"
#include "array.adt.h"

typedef enum {
   CtxcheckStatcodeOk,
   CtxcheckStatcodeErrcntMax
} ctxcheck_statcode_t;

typedef struct ctxcheck_ctx ctxcheck_ctx_t;
typedef struct ctxcheck_err_ctx ctxcheck_err_ctx_t;
typedef void ctxcheck_errhandler_t(
   ctxcheck_ctx_t *superctx,
   ctxcheck_err_ctx_t *errctx,
   size_t idx
);

struct ctxcheck_err_ctx {
   // const char *act_romnum;
   // const char *scene_romnum;
   // size_t ln, col;
   // const char *reason;
   // ctxcheck_errhandler_t *handle;
   tree_t *act, *scene;
   ctxcheck_errhandler_t *handle;
};

struct ctxcheck_ctx {
   const char *sourcefile;
   config_t *cfg;
   const char *reason;
   array_t *ls;
   array_t *errs;
   size_t errcnt, errcnt_max;
   ctxcheck_err_ctx_t err;
   tree_t *nrtv;
   tree_t *act, *scene;  /* for tree traversal */
   tree_t *t;  /* temporary tree */
   node_t *n;  /* temporary tree.dat */
};

typedef void ctxchecker_t(ctxcheck_ctx_t *octx);

static inline ctxchecker_t check_goto_unexistent;
static tree_callback_t check_goto_unexistent_route;
static inline ctxchecker_t check_goto_unexistent_act;
static inline ctxchecker_t check_goto_unexistent_scene;
static ctxchecker_t check_goto_unexistent_goto;

static ctxcheck_statcode_t check_empty_scene(ctxcheck_ctx_t *octx);

static ctxcheck_statcode_t regerr(ctxcheck_ctx_t *octx);
static void report_errors(ctxcheck_ctx_t *octx);
static ctxcheck_errhandler_t ctxcheck_errhandle_empty_scene;

static void print_err(ctxcheck_ctx_t *octx);
static void trace(ctxcheck_ctx_t *octx, tree_t *t, const char *type);
static void semerr_unknown_label(ctxcheck_ctx_t *octx, const char *s);

#endif
