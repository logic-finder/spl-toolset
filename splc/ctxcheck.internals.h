#ifndef CTXCHECK_INTERNALS_H
#define CTXCHECK_INTERNALS_H

#include <stdbool.h>

#include "msg.h"
#include "global.h"
#include "wrappers.h"
#include "arena.adt.h"
#include "array.adt.h"

typedef enum {
   CtxcheckStatOk,
   CtxcheckStatErr,
   CtxcheckStatErrcntMax,
   CtxcheckStatWarncntMax
} ctxcheck_statcode_t;

typedef struct ctxcheck_ctx ctxcheck_ctx_t;
typedef struct ctxcheck_err_ctx ctxcheck_err_ctx_t;
typedef void ctxcheck_errhandler_t(
   ctxcheck_ctx_t *superctx,
   ctxcheck_err_ctx_t *errctx,
   size_t idx
);

struct ctxcheck_err_ctx {
   tree_t *act;
   tree_t *scene;
   tree_t *curr;
   const char *reason;
   ctxcheck_errhandler_t *handle;
};

typedef ctxcheck_err_ctx_t ctxcheck_warn_ctx_t;
typedef ctxcheck_errhandler_t ctxcheck_warnhandler_t;

struct ctxcheck_ctx {
   const char *sourcefile;
   config_t *cfg;
   arena_t *arena;
   array_t *ls;
   array_t *errs, *warns;
   size_t errcnt, warncnt;
   size_t errcnt_max, warncnt_max;
   ctxcheck_err_ctx_t err;
   ctxcheck_warn_ctx_t warn;
   tree_t *dp;
   tree_t *nrtv;
   /* below members are for tree traversal */
   ctxcheck_statcode_t status;
   tree_t *act;
   tree_t *scene;
   tree_t *t;  /* current tree */
   node_t *n;  /* t.dat */
   size_t *dp_counters;
};

typedef struct {
   node_t *dat;
   line_t *ln;
} diag_info_t;

static bool route_act(ctxcheck_ctx_t *octx);
static bool route_scene(ctxcheck_ctx_t *octx);

static ctxcheck_statcode_t check_invalid_goto(ctxcheck_ctx_t *octx);
static tree_callback_t check_invalid_goto_route;
static bool check_invalid_goto_target(ctxcheck_ctx_t *octx);
static bool check_invalid_goto_target_act(ctxcheck_ctx_t *octx);
static bool check_invalid_goto_target_scene(ctxcheck_ctx_t *octx);

static ctxcheck_statcode_t check_empty_scene(ctxcheck_ctx_t *octx);

static ctxcheck_statcode_t check_empty_line(ctxcheck_ctx_t *octx);
static bool check_empty_line_route(tree_t *t, size_t lv, void *ctx);
static bool check_empty_line_target(ctxcheck_ctx_t *octx);

static ctxcheck_statcode_t check_unused_dp(ctxcheck_ctx_t *octx);
static bool check_unused_dp_route(tree_t *t, size_t lv, void *ctx);

static ctxcheck_statcode_t regerr(ctxcheck_ctx_t *octx);
static ctxcheck_statcode_t regwarn(ctxcheck_ctx_t *octx);
static void report_errors(ctxcheck_ctx_t *octx);
static void report_warnings(ctxcheck_ctx_t *octx);

static ctxcheck_warnhandler_t ctxcheck_warnhandle_unused_dp;

static ctxcheck_errhandler_t ctxcheck_errhandle_default;
static ctxcheck_errhandler_t ctxcheck_errhandle_invalid_goto;
static ctxcheck_errhandler_t ctxcheck_errhandle_empty_scene;

static inline void serr_header(size_t idx);
static inline void swarn_header(size_t idx);
static inline void under_act(ctxcheck_ctx_t *ctx, tree_t *act);
static inline void under_scene(ctxcheck_ctx_t *ctx, tree_t *scene);
static void under_template(const char *kind, ctxcheck_ctx_t *ctx, tree_t *t);
static void diag_template(const char *msg, ctxcheck_ctx_t *ctx, tree_t *t);
static diag_info_t get_di(ctxcheck_ctx_t *ctx, tree_t *t);

#endif
