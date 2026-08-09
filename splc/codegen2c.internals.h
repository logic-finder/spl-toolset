#ifndef CODEGEN2C_INTERNALS_H
#define CODEGEN2C_INTERNALS_H

#include "parse.h"
#include "irgen.h"
#include "global.h"
#include "tree.adt.h"
#include "wrappers.h"
#include "strutils.h"

static const char *indent = "   ";

static void codegen_title(FILE *fp);
static void codegen_locals(tree_t *irt_dp, FILE *fp, tree_t *pt_dp);
static void codegen_route(tree_t *t, int lv, void *ctx);

static void handle_scene(tree_t *t, FILE *fp);
static void handle_block(tree_t *t, FILE *fp);
static void handle_opcode(tree_t *t, FILE *fp);

static void codegen_set(tree_t *t, FILE *fp);
static void codegen_asgn(tree_t *t, FILE *fp);
static void codegen_enterlike(tree_t *t, FILE *fp, const char *op);
static void codegen_exeunt(FILE *fp);
static void codegen_speak(tree_t *t, FILE *fp);
static void codegen_push(FILE *fp);
static void codegen_pop(tree_t *t, FILE *fp);
static void codegen_binary_op(FILE *fp, const char *op);
static void codegen_unary_op(FILE *fp, const char *op);
static void codegen_twice(tree_t *t, FILE *fp);
static void codegen_io(FILE *fp, const char *op);
static void codegen_goto(tree_t *t, FILE *fp);
static void codegen_comp(FILE *fp, const char *op);
static void codegen_rememb(tree_t *t, FILE *fp);
static void codegen_recall(FILE *fp);
static void codegen_jump(tree_t *t, FILE *fp, bool v);
static void codegen_negate(FILE *fp);

static const char *resolve_var(irvar_t var);

#endif
