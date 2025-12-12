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
#define KEYWRD_ACT    "Act"
#define KEYWRD_SCENE  "Scene"
#define KEYWRD_ENTER  "Enter"
#define KEYWRD_EXIT   "Exit"
#define KEYWRD_EXEUNT "Exeunt"
#define KEYWRD_OPEN   "Open"
#define KEYWRD_SPEAK  "Speak"
#define KEYWRD_LISTEN "Listen"
#define KEYWRD_LET    "Let"
#define KEYWRD_WE     "We"
#define KEYWRD_IF     "If"
#define KEYWRD_REMEMB "Remember"
#define KEYWRD_RECALL "Recall"
#define KEYWRD_HEART  "heart"
#define KEYWRD_TO     "to"
#define KEYWRD_MIND   "mind"
#define KEYWRD_AND    "and"
#define KEYWRD_SO     "so"
#define KEYWRD_NOT    "not"
#define KEYWRD_THAN   "than"
#define KEYWRD_AS     "as"
#define KEYWRD_SUM    "sum"
#define KEYWRD_DIFF   "difference"
#define KEYWRD_PROD   "product"
#define KEYWRD_QUOT   "quotient"
#define KEYWRD_REM    "remainder"
#define KEYWRD_SQUR   "square"
#define KEYWRD_ROOT   "root"
#define KEYWRD_CUBE   "cube"
#define KEYWRD_2X     "twice"
#define KEYWRD_FACT   "factorial"
#define KEYWRD_OF     "of"
#define KEYWRD_BTW    "between"
#define KEYWRD_I      "I"
#define KEYWRD_MY     "my"
#define KEYWRD_YOU    "You"
#define KEYWRD_THOU   "Thou"
#define KEYWRD_YOU_L  "you"
#define KEYWRD_THOU_L "thou"
#define KEYWRD_YOUR   "your"
#define KEYWRD_YOUR_U "YOUR"
#define KEYWRD_THY    "thy"
#define KEYWRD_THINE  "thine"
#define KEYWRD_HIS    "his"
#define KEYWRD_HER    "her"
#define KEYWRD_ITS    "its"
#define KEYWRD_A      "a"
#define KEYWRD_AN     "an"
#define KEYWRD_THE    "the"
#define KEYWRD_AM     "Am"
#define KEYWRD_ARE    "are"
#define KEYWRD_ART    "art"
#define KEYWRD_ARE_C  "Are"
#define KEYWRD_ART_C  "Art"
#define KEYWRD_IS     "Is"
#define KEYWRD_US     "us"
#define KEYWRD_SHALL  "shall"
#define KEYWRD_MUST   "must"
#define KEYWRD_RETURN "return"
#define KEYWRD_PROCED "proceed"

/************
 * TYPEDEFS *
 ************/
typedef int seeker_t(void);
typedef void parser_t(void);

typedef struct stmthandler {
   seeker_t *seek;
   parser_t *parse;
} stmthandler_t;

typedef void operator_template_t(
   tree_t *op,
   const char * restrict type,
   const char * restrict err
);
typedef void operator_t(tree_t *op);

/***********************
 * FUNCTION PROTOTYPES *
 ***********************/
/* Seekers */
static void seek_stmt(void);
static void seek_stmt_router(void);
static int seek_enterlike(const char *type);
static nodekind_t seek_op(void);
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
static int parse_stmt(void);
static int parse_line_as_conseq(void);
static void parse_const(tree_t *stmt);
static void parse_op(tree_t *stmt, nodekind_t kind);
static int parse_line_router(
   const stmthandler_t stmts[static 6],
   int stmts_len
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
static parser_t parse_asgn_i;
static parser_t parse_asgn_ii;
static parser_t parse_out;
static parser_t parse_in;
static parser_t parse_goto;
static parser_t parse_cond;
static parser_t parse_if;
static parser_t parse_push;
static parser_t parse_pop;
static operator_template_t parse_op_unary;
static operator_template_t parse_op_binary;
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

/* Token Handling */
static void gettok(void);
static void gettokn(int n);
static void ungettok(void);
static void ungettokn(int n);
static void skiptoks(char sentinel);
static void skiptoks2(const char *sentinels);
static void eqtok(char ch);
static void neqtok(char ch);
static void readtoks(char sentinel, tree_t *base);
static void nexttok(void);

/* Utils */
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
   nodekind_t kind,
   int lnum,
   int lpos
);
static tree_t *graft_tree(
   tree_t *base,
   const char *run,
   int len,
   nodekind_t kind
);

#endif
