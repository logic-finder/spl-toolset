#ifndef IRGEN_INTERNALS_H
#define IRGEN_INTERNALS_H

#include "global.h"
#include "wrappers.h"
#include "tree.adt.h"

typedef void handler_t(tree_t *t);

static void set_dpsz(void);
static tree_callback_t route;
static handler_t handle_act;
static handler_t handle_scene;
static handler_t handle_enter;
static handler_t handle_exit;
static handler_t handle_exeunt;
static handler_t handle_line;
static handler_t handle_asgn;
static void handle_io(tree_t *t, iropcode_t inst);
static handler_t handle_goto;
static handler_t handle_cond;
static handler_t handle_if;
static handler_t handle_push;
static handler_t handle_pop;
static void resolve_const(tree_t *t);
static void resolve_unary_op(tree_t *t, iropcode_t inst);
static void resolve_binary_op(tree_t *t, iropcode_t inst);
static void resolve_noun(tree_t *t);
static void set_operands(void);

static tree_t *plant_tree(
   const char *run,
   size_t len,
   irnodekind_t kind,
   int lnum,
   int lpos
);

static tree_t *graft_tree_s(
   tree_t *base,
   const char *run,
   size_t len,
   irnodekind_t kind,
   size_t lnum,
   size_t lpos
);

static tree_t *graft_tree_n(
   tree_t *base,
   int val,
   irnodekind_t kind,
   size_t lnum,
   size_t lpos
);

static void add_block(int n);

tree_t *irt;
static tree_t *curr_act, *curr_scene, *curr_block;
static int labelcnt;

#endif
