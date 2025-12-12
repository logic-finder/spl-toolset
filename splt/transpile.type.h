#ifndef TRANSPILE_TYPE_H
#define TRANSPILE_TYPE_H

#include "global.h"
#include "strutil.h"
#include "wrapper.h"
#include "transpile.h"

#define TEMPFILE_MAIN_NAME "__splt_temp_main__"
#define TEMPFILE_ACT_NAME  "__splt_temp_act__"

typedef void generator_t(tree_t *t);

static void transpile2C(tree_t *pt);

static tree_callback_t generate_c_router;

static generator_t gen_title;
static generator_t gen_dp;
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

static void resolve_const(tree_t *t);

#endif
