#ifndef PARSE_TYPE_H
#define PARSE_TYPE_H

#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include "msg.h"
#include "lex.h"
#include "global.h"
#include "wrapper.h"
#include "strutil.h"
#include "tree.adt.h"
#include "colorcode.h"

/**********
 * MACROS *
 **********/
#define LONGJMP_ENV env_parse_stmt
#define JUMP(v) LONGJMP_TEMPLET(LONGJMP_ENV, (v))
#
#define TREE_CHDAT(t,i) ((node_t *) tree_chdat((t), (i)))

/************
 * TYPEDEFS *
 ************/
typedef enum stmtkind {
   STMTKIND_ENTER = 1,
   STMTKIND_EXIT,
   STMTKIND_EXEUNT,
   STMTKIND_LINE,
   STMTKIND_ACT,
   STMTKIND_SCENE,
   STMTKIND_FINALE
} stmtkind_t;

typedef enum opkind {
   OPKIND_NAO,  /* not an operator */
   OPKIND_SUM,
   OPKIND_DIFF,
   OPKIND_PROD,
   OPKIND_QUOT,
   OPKIND_REM,
   OPKIND_SQRT,
   OPKIND_SQUR,
   OPKIND_CUBE,
   OPKIND_2X,
   OPKIND_FACT
} opkind_t;

typedef int seeker_t(void);
typedef void parser_t(void);

typedef struct stmthandler {
   seeker_t *seek;
   parser_t *parse;
} stmthandler_t;

/***********************
 * FUNCTION PROTOTYPES *
 ***********************/
/* Seekers */
static void seek_stmt(void);
static void seek_stmt_router(void);
static opkind_t seek_op(void);
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
static char *get_opname(opkind_t kind);
static int parse_stmt(void);
static void parse_const(tree_t *stmt);
static void parse_op(tree_t *stmt, opkind_t kind);
typedef void operator_template_t(tree_t *op, const char *type, const char *err);
static operator_template_t parse_op_unary;
static operator_template_t parse_op_binary;
typedef void operator_t(tree_t *op);
static operator_t parse_op_sum;
static operator_t parse_op_diff;
static operator_t parse_op_prod;
static operator_t parse_op_quot;
static operator_t parse_op_rem;
static operator_t parse_op_sqrt;
static operator_t parse_op_squr;
static operator_t parse_op_cube;
static operator_t parse_op_2x;
static operator_t parse_op_fact;
//static void parse_op_operand(tree_t *stmt, const char *type, int ret);
static int parse_line_as_conseq(void);
static int parse_line_router(
   const stmthandler_t stmts[static 6],
   int stmts_len);
static parser_t parse_title;
static parser_t parse_dp;
static parser_t parse_act;
static parser_t parse_scene;
static parser_t parse_enter;
static parser_t parse_exit;
static parser_t parse_exeunt;
static parser_t parse_line;
static parser_t parse_asgn;
static parser_t parse_asgn_i;
static parser_t parse_asgn_ii;
static parser_t parse_out;
static parser_t parse_in;
static parser_t parse_goto;
static parser_t parse_cond;
static parser_t parse_if;
static parser_t parse_push;
static parser_t parse_pop;

/* Token Handling */
static void gettok(void);
static void gettokn(int n);
static void ungettok(void);
static void ungettokn(int n);
static void skiptoks(char sentinel);
static void skiptoks2(char *sentinels);
static void eqtok(char ch);
static void neqtok(char ch);
static void readtoks(char sentinel, tree_t *base);
static void nexttok(void);

/* Utils */
static int match_tokrun(const char **arr, int len);
static inline void archive_tokstate(void);
static inline void rewind_tokstate(void);

/* Error Handling */
static inline void synerr(void);
static teller_t tell;

/* Miscellnaeous */
static arr_iterator_t cleanup_tokstream;
static tree_t *plant_tree(
   const char *run,
   int len,
   const char *tag,
   int lnum,
   int lpos
);
static tree_t *graft_tree(
   tree_t *base,
   const char *run,
   int len,
   const char *tag
);
static void rewrite_ll(tree_t *t);

#endif
