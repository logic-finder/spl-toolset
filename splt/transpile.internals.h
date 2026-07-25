#ifndef TRANSPILE_INTERNALS_H
#define TRANSPILE_INTERNALS_H

#include <stdio.h>
#include "global.h"
#include "wrapper.h"
#include "tree.adt.h"

#define FP_NAME "temp.c"
#define INDENT  "   "

typedef void generator_t(tree_t *t);
typedef void resolver_t(tree_t *t);

static void generate(tree_t *t, int _);
static void gen_header(void);
static void gen_title(tree_t *title);
static void gen_locals(tree_t *dp);
static void gen_cleanup(void);
static void gen_io(const char *kind);
static generator_t gen_act;
static generator_t gen_scene;
static generator_t gen_enter;
static generator_t gen_exit;
static generator_t gen_exeunt;
static generator_t gen_line;
static generator_t gen_asgn;
static generator_t gen_outn;
static generator_t gen_outc;
static generator_t gen_inn;
static generator_t gen_inc;
static generator_t gen_goto;
static generator_t gen_cond;
static generator_t gen_if;
static generator_t gen_push;
static generator_t gen_pop;

static void eval_const(tree_t *cnst);
static void resolve_division(tree_t *op, const char *s);
static void resolve_unary(tree_t *op, const char *s);
static void resolve_binary(tree_t *op, char c);
static resolver_t resolve_noun;
static resolver_t resolve_sqrt;
static resolver_t resolve_squr;
static resolver_t resolve_cube;
static resolver_t resolve_2x;
static resolver_t resolve_fact;
static resolver_t resolve_sum;
static resolver_t resolve_diff;
static resolver_t resolve_prod;
static resolver_t resolve_quot;
static resolver_t resolve_rem;

#endif
