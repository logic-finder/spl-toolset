#ifndef IRGEN_INTERNALS_H
#define IRGEN_INTERNALS_H

#include "global.h"
#include "wrappers.h"
#include "tree.adt.h"

typedef struct {
   tree_t
      *irt,
      *nrtv,
      *curr_act,
      *curr_scene,
      *curr_block;
   size_t labelcnt;
} irgen_ctx_t;

typedef void handler_t(tree_t *t, irgen_ctx_t *ictx);

static void set_dpsz(irgen_ctx_t *ictx);
static tree_callback_t route;
static handler_t handle_act;
static handler_t handle_scene;
static handler_t handle_enter;
static handler_t handle_exit;
static handler_t handle_exeunt;
static handler_t handle_line;
static handler_t handle_asgn;
static void handle_io(tree_t *t, irgen_ctx_t *ictx, iropcode_t opcode);
static handler_t handle_goto;
static handler_t handle_cond;
static handler_t handle_if;
static handler_t handle_push;
static handler_t handle_pop;
static void resolve_const(tree_t *t, irgen_ctx_t *ictx);
static void resolve_unary_op(tree_t *t, irgen_ctx_t *ictx, iropcode_t opcode);
static void resolve_binary_op(tree_t *t, irgen_ctx_t *ictx, iropcode_t opcode);
static void resolve_noun(tree_t *t, irgen_ctx_t *ictx);
static void set_operands(irgen_ctx_t *ictx);

static tree_t *plant_tree(
   const char *run,
   size_t len,
   irnodekind_t kind,
   size_t lnum,
   size_t lpos
);

static tree_t *graft_tree_s(
   tree_t *base,
   const char *run,
   size_t len,
   irnodekind_t kind,
   size_t lnum,
   size_t lpos
);
static tree_t *graft_tree_i(tree_t *base, int i, irnodekind_t kind);
static tree_t *graft_tree_ui(tree_t *base, unsigned int ui, irnodekind_t kind);
static tree_t *graft_tree_opcode(
   tree_t *base,
   iropcode_t opcode,
   size_t lnum,
   size_t lpos
);
static tree_t *graft_tree_var(tree_t *base, irvar_t var);
static tree_t *graft_tree_charidx(tree_t *base, spl_uint_t charidx);
static inline tree_t *graft_tree_common(
   irnode_t *node,
   size_t lnum,
   size_t lpos
);

static void add_block(irgen_ctx_t *ictx, size_t cnt);

tree_t *irt;

#endif
