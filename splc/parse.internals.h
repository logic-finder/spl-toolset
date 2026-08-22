#ifndef PARSE_INTERNALS_H
#define PARSE_INTERNALS_H

#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>

#include "msg.h"
#include "lex.h"
#include "common.h"
#include "global.h"
#include "argparse.h"
#include "wrappers.h"
#include "strutils.h"
#include "tree.adt.h"
#include "colorcode.h"
#include "dbhandler.h"

/*==========*
 | TYPEDEFS |
 *==========*/
typedef struct {
   optflg_t *of;  /* option flags */
   optval_t *ov;  /* option values */
   array_t *ls;  /* array of line_t */
   array_t *toks;  /* token stream */
   token_t *tok;   /* toks[idx] */
   token_t *etok;  /* used in synerr() to print a error */
   size_t len;     /* toks.length */
   size_t idx;     /* current index in toks */
   tree_t *pt;     /* parse tree */
   tree_t *nrtv;   /* contains the whole narrative */
   tree_t *act;    /* current act */
   tree_t *scene;  /* current scene */
   tree_t *line;   /* current line */
   const char *reason;   /* the reason of a syntax error */
   jmp_buf env;     /* used in parse_stmt() */
   size_t charidx;  /* updated by is_name() */
   size_t be_kind;  /* updated by is_be_conjs() */
} parse_ctx_t;

typedef int seeker_t(parse_ctx_t *pctx);
typedef void parser_t(parse_ctx_t *pctx);

typedef struct stmthandler {
   seeker_t *seek;
   parser_t *parse;
} stmthandler_t;

typedef void operator_t(parse_ctx_t *pctx, tree_t *op);

/*=====================*
 | FUNCTION PROTOTYPES |
 *=====================*/
static tree_callback_t cleanup_node;
static size_t count_tree_node(tree_t *root);
static tree_callback_t shownode;
static const char *resolve_nodekind(nodekind_t kind);

/* Seekers */
static void seek_stmt(parse_ctx_t *pctx);
static void seek_stmt_router(parse_ctx_t *pctx);
static int seek_enterlike(parse_ctx_t *pctx, const char *type);
static nodekind_t seek_op(parse_ctx_t *pctx);
static seeker_t seek_act;
static seeker_t seek_scene;
static seeker_t seek_enter;
static seeker_t seek_exit;
static seeker_t seek_exeunt;
static seeker_t seek_line;
static seeker_t seek_asgn;
static seeker_t seek_out;
static seeker_t seek_in;
static seeker_t seek_goto;
static seeker_t seek_cond;
static seeker_t seek_if;
static seeker_t seek_push;
static seeker_t seek_pop;

/* Parsers */
static int parse_stmt(parse_ctx_t *pctx);
static void parse_namelist(parse_ctx_t *pctx, tree_t *t);
static bool parse_line_as_conseq(parse_ctx_t *pctx);
static void parse_const(parse_ctx_t *pctx, tree_t *stmt);
static void parse_noun_phrase(parse_ctx_t *pctx, tree_t *cnst);
static void parse_cond_eq(parse_ctx_t *pctx, tree_t *cond);
static void parse_cond_ineq(parse_ctx_t *pctx, tree_t *cond);
static void parse_op(parse_ctx_t *pctx, tree_t *stmt, nodekind_t kind);
static bool parse_line_router(
   parse_ctx_t *pctx, const stmthandler_t *table, size_t tsiz
);
static parser_t parse_title;
static parser_t parse_dp;
static parser_t parse_act;
static parser_t parse_scene;
static parser_t parse_enter;
static parser_t parse_exit;
static parser_t parse_exeunt;
static parser_t parse_line;
static parser_t parse_asgn;
static tree_t *parse_asgn_i(parse_ctx_t *pctx, token_t *you);
static tree_t *parse_asgn_ii(parse_ctx_t *pctx, token_t *you);
static tree_t *parse_asgn_iii(parse_ctx_t *pctx, token_t *you);
static parser_t parse_out;
static parser_t parse_in;
static parser_t parse_goto;
static parser_t parse_cond;
static parser_t parse_if;
static parser_t parse_push;
static parser_t parse_pop;
static void parse_op_unary(
   parse_ctx_t *pctx,
   tree_t * restrict op,
   const char * restrict err
);
static void parse_op_binary(
   parse_ctx_t *pctx,
   tree_t * restrict op,
   const char * restrict type,
   const char * restrict err
);
static inline operator_t parse_op_sum;
static inline operator_t parse_op_diff;
static inline operator_t parse_op_prod;
static inline operator_t parse_op_quot;
static inline operator_t parse_op_rem;
static inline operator_t parse_op_sqrt;
static inline operator_t parse_op_squr;
static inline operator_t parse_op_cube;
static inline operator_t parse_op_2x;
static inline operator_t parse_op_fact;

/* Token Handling */
static void gettok(parse_ctx_t *pctx);
static void gettokn(parse_ctx_t *pctx, size_t n);
static void ungettok(parse_ctx_t *pctx);
static void ungettokn(parse_ctx_t *pctx, size_t n);
static void skiptoks2(parse_ctx_t *pctx, const char *sentinels);
static void eqtok(parse_ctx_t *pctx, char ch);
static void neqtok(parse_ctx_t *pctx, char ch);
static void readtoks(parse_ctx_t *pctx, char sentinel, tree_t *base);
static void readtoks_until(parse_ctx_t *pctx, char *scanset, tree_t *t);
static void nexttok(parse_ctx_t *pctx);

/* Utils */
static bool is_name(parse_ctx_t *pctx);
static bool is_name_lower(parse_ctx_t *pctx);
static bool is_pronoun(const char *str);
static bool is_reflexive(const char *str);
static bool is_nil(const char *str);
static bool is_article(const char *str);
static bool is_possessive(const char *str);
static bool is_be_conjs(parse_ctx_t *pctx);

static nodekind_t what_pronoun(const char *str);
static nodekind_t what_reflexive(const char *str);

static void check_const_end(parse_ctx_t *pctx);
static void check_cond_predicate(parse_ctx_t *pctx);
static void check_asgn_predicate(parse_ctx_t *pctx);

static inline void rewind_tokstate(parse_ctx_t *pctx, size_t orig_idx);
static inline void check_eoe(parse_ctx_t *pctx);

/* Error Handling */
static void synwarn(parse_ctx_t *pctx);
static void synerr(parse_ctx_t *pctx);

/* Miscellnaeous */
static tree_t *plant_tree(
   const char *run,
   size_t len,
   nodekind_t kind,
   size_t lnum,
   size_t lpos
);
static tree_t *graft_tree_s(
   tree_t *base,
   nodekind_t kind,
   const token_t *tok
);
static tree_t *graft_tree_n(
   tree_t *base,
   int val,
   nodekind_t kind,
   const token_t *tok
);

/*==================*
 | GLOBAL VARIABLES |
 *==================*/
static const stmthandler_t stmt_hdlrs[] = {
   { seek_if   , parse_if   },  /* MUST be 1st! see `parse_line_as_conseq` */
   { seek_asgn , parse_asgn },
   { seek_out  , parse_out  },
   { seek_in   , parse_in   },
   { seek_goto , parse_goto },
   { seek_cond , parse_cond },
   { seek_push , parse_push },
   { seek_pop  , parse_pop  }
};

#endif
