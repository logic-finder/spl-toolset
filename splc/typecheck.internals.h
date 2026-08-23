#ifndef TYPECHECK_INTERNALS_H
#define TYPECHECK_INTERNALS_H

#include "msg.h"
#include "parse.h"
#include "common.h"
#include "global.h"
#include "wrappers.h"
#include "strutils.h"
#include "dbhandler.h"
#include "colorcodes.h"

typedef struct {
   const char *reason;
   tree_t *pt, *title, *dp;
   optval_t *ov;
   array_t *ls;
   tree_t *t;  /* temporary tree */
   node_t *n;  /* temporary tree.dat */
} typecheck_ctx_t;

typedef void typechecker_t(typecheck_ctx_t *tctx);

static void coalesce_childstr(tree_t *t);
static inline void coalesce_title(typecheck_ctx_t *tctx);
static void coalesce_name(typecheck_ctx_t *tctx);
static void check_namecol(typecheck_ctx_t *tctx);

static tree_callback_t typecheck_router;
static typechecker_t typecheck_name;
static typechecker_t typecheck_adj;
static typechecker_t typecheck_noun;
static typechecker_t typecheck_comp;
static typechecker_t typecheck_rnum;

static int is_pronoun(const char *s);

static void setnds(node_t *n, char *s, int l);

static inline void print_errheader(void);
static void semerr_badword(typecheck_ctx_t *tctx);
static void semerr_dupname(
   typecheck_ctx_t *tctx,
   node_t * restrict curr,
   node_t * restrict prev
);

#endif
