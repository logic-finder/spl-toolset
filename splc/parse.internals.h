#ifndef PARSE_INTERNALS_H
#define PARSE_INTERNALS_H

#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>

#include "msg.h"
#include "lex.h"
#include "global.h"
#include "wrappers.h"
#include "strutils.h"
#include "tree.adt.h"
#include "colorcode.h"
#include "dbhandler.h"

/**********
 * MACROS *
 **********/
#define LONGJMP_ENV env_parse_stmt
#define JUMP(v) LONGJMP_TEMPLET(LONGJMP_ENV, (v))

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
static void parse_namelist(tree_t *t);
static int parse_line_as_conseq(void);
static void parse_const(tree_t *stmt);
static void parse_cond_eq(tree_t *cond);
static void parse_cond_ineq(tree_t *cond);
static void parse_op(tree_t *stmt, nodekind_t kind);
static int parse_line_router(
   const stmthandler_t stmts[8],
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
static tree_t *parse_asgn_i(token_t *you);
static tree_t *parse_asgn_ii(token_t *you);
static tree_t *parse_asgn_iii(token_t *you);
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
static bool is_pronoun(const char *str);
static bool is_reflexive(const char *str);
static bool is_nil(const char *str);
static bool is_article(const char *str);
static bool is_possessive(const char *str);
static nodekind_t what_pronoun(const char *str);
static nodekind_t what_reflexive(const char *str);
static void check_const_end(void);

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
static int isname(void);
static int isname_lower(void);
static inline void archive_tokstate(void);
static inline void rewind_tokstate(void);

/* Error Handling */
static inline void synerr(void);

/* Miscellnaeous */
static array_iterator_t cleanup_tokstream;
static tree_t *plant_tree(
   const char *run,
   int len,
   nodekind_t kind,
   int lnum,
   int lpos
);
static tree_t *graft_tree_s(
   tree_t *base,
   const char *run,
   int len,
   nodekind_t kind
);
static tree_t *graft_tree_n(
   tree_t *base,
   int val,
   nodekind_t kind
);

/******************************
 * IMPORTANT GLOBAL VARIABLES *
 ******************************/
/* Token Stream */
static array_t *toks;
static token_t
   *tok,    // toks[idx]
   *etok;   // used in `tell()` for printing an error
static int
   len,     // toks.length
   idx,     // current index in toks
   tidx;    // temp. var. for idx

/* Parse Tree */
extern tree_t
   *pt;     // parse tree (see global.h)
static tree_t
   *nrtv,   // contains the whole narrative
   *act,    // current act
   *scene,  // current scene
   *line;   // current line

/* Miscellaneous */

// extern const char *sfname;   // see global.h
// extern msg_t msgs;           // see global.h

static const char *reason;   // error message
static jmp_buf LONGJMP_ENV;  // for setjmp & longjmp
static int charidx;          // used by `isname` & its caller

/*
 * `seek_if` and `parse_if` being the
 * first element is intentional;
 * refer to `parse_line_as_conseq`.
 */
static const stmthandler_t stmts[] = {
   { seek_asgn , parse_asgn },
   { seek_out  , parse_out  },
   { seek_in   , parse_in   },
   { seek_goto , parse_goto },
   { seek_cond , parse_cond },
   { seek_if   , parse_if   },
   { seek_push , parse_push },
   { seek_pop  , parse_pop  }
};
static const int stmts_len = ARRLEN(stmts);

/* These are used in `seek_cond` and `parse_cond` only. */
static const char *cond_verbs[] = {
   KEYWRD_AM, KEYWRD_ARE_C, KEYWRD_ART_C, KEYWRD_IS
};
static const int cond_verbs_len = ARRLEN(cond_verbs);
static const char *cond_subjs[] = {
   KEYWRD_I, KEYWRD_YOU_L, KEYWRD_THOU_L
};
static const int cond_subjs_len = ARRLEN(cond_subjs);
static int vtype;

#endif
