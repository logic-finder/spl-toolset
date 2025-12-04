#ifndef PARSE_TYPE_H
#define PARSE_TYPE_H

#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include "msg.h"
#include "lex.h"
#include "tree.h"
#include "global.h"
#include "wrapper.h"
#include "strutil.h"
#include "colorcode.h"

// #define RET_BOL  0  // beginning of line
// #define RET_EOT  1  // end of statement
// #define RET_EOS  2  // end of scene
// #define RET_EOA  3  // end of act
// #define RET_EOE  4  // end of everything

#define NEXT_ENTER   1
#define NEXT_EXIT    2
#define NEXT_EXEUNT  3
#define NEXT_LINE    4
#define NEXT_ACT     5
#define NEXT_SCENE   6
#define NEXT_FINALE  7

typedef int seeker_t(void);
typedef int parser_t(void);
typedef void parser_t2(void);

typedef struct finalehandler {
      seeker_t *seek;
      int retval;
   } finalehandler_t;

typedef struct stmthandler {
   seeker_t *seek;
   parser_t2 *parse;
} stmthandler_t;

static parser_t2 parse_title;
static parser_t2 parse_dp;
static parser_t2 parse_act;
static parser_t2 parse_scene;
static parser_t2 parse_stmt;
static parser_t2 parse_enter;
static parser_t2 parse_exit;
static parser_t2 parse_exeunt;
static void parse_const(tree_t *stmt);
static void parse_op(tree_t *stmt);
static void parse_op_operand(tree_t *stmt, const char *type);
static parser_t2 parse_line;
static void parse_line_as_consequent(void);
static parser_t2 parse_asgn;
static parser_t2 parse_asgn_i;
static parser_t2 parse_asgn_ii;
static parser_t2 parse_out;
static parser_t2 parse_in;
static parser_t2 parse_goto;
static parser_t2 parse_cond;
static parser_t2 parse_if;
static parser_t2 parse_push;
static parser_t2 parse_pop;

static void seek_stmt(void);
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

static void postprocess_dp(void);

static void synerr(teller_t *tell);
static teller_t tell;

static void gettok(void);
static void ungettok(void);
static void skiptok(char sentinel);
static void testtok(char sentinel);
static void readtok(char sentinel, tree_t *base);

static int match_tokdat(const char **arr, int len);

static inline void validate(tree_t *type);

static inline void archive_tokstate(void);
static inline void rewind_tokstate(void);

#endif
