#ifndef IRDUMP_INTERNALS_H
#define IRDUMP_INTERNALS_H

#include "parse.h"
#include "irgen.h"
#include "global.h"
#include "strutils.h"
#include "wrappers.h"

#define INDENT "    "

typedef struct {
   FILE *fp;
   optflg_t *of;
   tree_t *irt, *nrtv;
   tree_t *t;
   irnode_t *n;
   const char *curr_act;
} irdump_ctx_t;

typedef void handler_t(irdump_ctx_t *ictx);

static tree_callback_t route;
static handler_t handle_opcode;
static handler_t handle_act;
static handler_t handle_scene;
static handler_t handle_block;
static handler_t handle_setlike;
static handler_t handle_enterlike;
static handler_t handle_pushlike;
static handler_t handle_goto;
static handler_t handle_jumplike;
static handler_t handle_binary_op;
static handler_t handle_unary_op;
static handler_t handle_paramless_opcode;
static inline void emit_debug_data(irdump_ctx_t *ictx);
static const char *resolve_opcode(iropcode_t opcode);
static const char *resolve_var(irvar_t var);
static const char *resolve_nodekind(irnodekind_t kind);

#endif
