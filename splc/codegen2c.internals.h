#ifndef CODEGEN2C_INTERNALS_H
#define CODEGEN2C_INTERNALS_H

#include "parse.h"
#include "irgen.h"
#include "global.h"
#include "tree.adt.h"
#include "wrappers.h"
#include "lib/strutils.h"

typedef struct {
   FILE *fp;
   tree_t *irt, *dp, *nrtv;
   tree_t *pt, *title, *pt_dp;
   tree_t *t;
   irnode_t *n;
   const char *ferr;
} cg2c_ctx_t;

static const char *indent = "   ";

static bool codegen_title(cg2c_ctx_t *gctx);
static bool codegen_locals(cg2c_ctx_t *gctx);
static tree_callback_t codegen_route;

static bool handle_scene(cg2c_ctx_t *gctx);
static bool handle_block(cg2c_ctx_t *gctx);
static bool handle_opcode(cg2c_ctx_t *gctx);

static bool codegen_set(cg2c_ctx_t *gctx);
static bool codegen_asgn(cg2c_ctx_t *gctx);
static bool codegen_enterlike(cg2c_ctx_t *gctx, const char *op);
static bool codegen_exeunt(cg2c_ctx_t *gctx);
static bool codegen_speak(cg2c_ctx_t *gctx);
static bool codegen_push(cg2c_ctx_t *gctx);
static bool codegen_pop(cg2c_ctx_t *gctx);
static bool codegen_binary_op(cg2c_ctx_t *gctx, const char *op);
static bool codegen_unary_op(cg2c_ctx_t *gctx, const char *op);
static bool codegen_twice(cg2c_ctx_t *gctx);
static bool codegen_io(cg2c_ctx_t *gctx, const char *op);
static bool codegen_goto(cg2c_ctx_t *gctx);
static bool codegen_comp(cg2c_ctx_t *gctx, const char *op);
static bool codegen_rememb(cg2c_ctx_t *gctx);
static bool codegen_recall(cg2c_ctx_t *gctx);
static bool codegen_jump(cg2c_ctx_t *gctx, bool v);
static bool codegen_negate(cg2c_ctx_t *gctx);

static const char *resolve_var(cg2c_ctx_t *gctx, irvar_t var);

#endif
