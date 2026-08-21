#ifndef CODEGEN2C_INTERNALS_H
#define CODEGEN2C_INTERNALS_H

#include "parse.h"
#include "irgen.h"
#include "global.h"
#include "tree.adt.h"
#include "wrappers.h"
#include "strutils.h"

typedef struct {
   FILE *fp;
   tree_t *irt, *dp, *nrtv;
   tree_t *pt, *title, *pt_dp;
   tree_t *t;
   irnode_t *n;
} cg2c_ctx_t;

static const char *indent = "   ";

static void codegen_title(cg2c_ctx_t *gctx);
static void codegen_locals(cg2c_ctx_t *gctx);
static void codegen_route(tree_t *t, int lv, void *ctx);

static void handle_scene(cg2c_ctx_t *gctx);
static void handle_block(cg2c_ctx_t *gctx);
static void handle_opcode(cg2c_ctx_t *gctx);

static void codegen_set(cg2c_ctx_t *gctx);
static void codegen_asgn(cg2c_ctx_t *gctx);
static void codegen_enterlike(cg2c_ctx_t *gctx, const char *op);
static void codegen_exeunt(cg2c_ctx_t *gctx);
static void codegen_speak(cg2c_ctx_t *gctx);
static void codegen_push(cg2c_ctx_t *gctx);
static void codegen_pop(cg2c_ctx_t *gctx);
static void codegen_binary_op(cg2c_ctx_t *gctx, const char *op);
static void codegen_unary_op(cg2c_ctx_t *gctx, const char *op);
static void codegen_twice(cg2c_ctx_t *gctx);
static void codegen_io(cg2c_ctx_t *gctx, const char *op);
static void codegen_goto(cg2c_ctx_t *gctx);
static void codegen_comp(cg2c_ctx_t *gctx, const char *op);
static void codegen_rememb(cg2c_ctx_t *gctx);
static void codegen_recall(cg2c_ctx_t *gctx);
static void codegen_jump(cg2c_ctx_t *gctx, bool v);
static void codegen_negate(cg2c_ctx_t *gctx);

static const char *resolve_var(irvar_t var);

#endif
