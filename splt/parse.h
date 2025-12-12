#ifndef PARSE_H
#define PARSE_H

#include "arr.adt.h"
#include "tree.adt.h"
#include "argparse.h"

#define TREE_CHDAT(t,i) ((node_t *) tree_chdat((t),(i)))

typedef enum nodekind {
   /* 000 Misc. */
   NODEKIND__SETJMP = 0,  /* must be 0 */
   NODEKIND__FINALE,
   NODEKIND__NAO,

   /* 100 General */
   NODEKIND_ROOT = 100,
   NODEKIND_DATA,
   NODEKIND_ADJ,
   NODEKIND_NOUN,
   NODEKIND_AFFIRM,
   NODEKIND_NEGATE,
   NODEKIND_LHS,
   NODEKIND_RHS,
   NODEKIND_PERSON,
   NODEKIND_EQ,
   NODEKIND_INEQ,

   /* 200 Structure */
   NODEKIND_TITLE = 200,
   NODEKIND_DP,
   NODEKIND_CHAR,
   NODEKIND_ACT,
   NODEKIND_SCENE,
   NODEKIND_ENTER,
   NODEKIND_EXIT,
   NODEKIND_EXEUNT,
   NODEKIND_LINE,
   NODEKIND_ASSIGN,
   NODEKIND_OUT_N,
   NODEKIND_OUT_C,
   NODEKIND_IN_N,
   NODEKIND_IN_C,
   NODEKIND_GOTO,
   NODEKIND_COND,
   NODEKIND_IF,
   NODEKIND_PUSH,
   NODEKIND_POP,

   /* 300 Operator */
   NODEKIND_SUM = 300,
   NODEKIND_DIFF,
   NODEKIND_PROD,
   NODEKIND_QUOT,
   NODEKIND_REM,
   NODEKIND_SQRT,
   NODEKIND_SQUR,
   NODEKIND_CUBE,
   NODEKIND_2X,
   NODEKIND_FACT
} nodekind_t;

typedef struct node {
   char *run;
   int len;
   nodekind_t kind;
   int lnum, lpos;
} node_t;

tree_t *parse(
   optflg_t *of,
   optval_t *ov,
   arr_t *tokens
);

#endif
