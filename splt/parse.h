#ifndef PARSE_H
#define PARSE_H

#include "arr.adt.h"
#include "tree.adt.h"
#include "argparse.h"

#define TREE_CHDAT(t,i) ((node_t *) tree_chdat((t),(i)))

typedef enum nodekind {
   /* 000 Misc. */
   NODEKIND__SETJMP = 0,  /* this MUST be 0 */
   NODEKIND__FINALE,
   NODEKIND__NAO,

   /* 100 General */
   NODEKIND_ROOT = 100,
   NODEKIND_DATA,
   NODEKIND_ROMNUM,
   NODEKIND_SUBJ,
   NODEKIND_CONST,
   NODEKIND_ADJ,
   NODEKIND_NOUN,
   NODEKIND_PNOUN,
   NODEKIND_NNOUN,
   NODEKIND_AFFIRM,
   NODEKIND_NEGATE,
   NODEKIND_CONSEQ,
   NODEKIND_LHS,
   NODEKIND_RHS,
   NODEKIND_P1,
   NODEKIND_P2,
   NODEKIND_P3,
   NODEKIND_EQ,
   NODEKIND_INEQ,
   NODEKIND_GT,
   NODEKIND_LT,

   /* 200 Structure */
   NODEKIND_TITLE = 200,
   NODEKIND_DP,
   NODEKIND_CHDECL,
   NODEKIND_NRTV,
   NODEKIND_ACT,
   NODEKIND_SCENE,
   NODEKIND_ENTER,
   NODEKIND_EXIT,
   NODEKIND_EXEUNT,
   NODEKIND_CHAR,
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

/*
 * `dat` - an union; either an int or a string structure
 * `dat.n` - treats the union as an int value
 * `dat.s` - treats the union as a string structure
 * `dat.s.run` - a string
 * `dat.s.len` - the length of the string
 * `kind` - kind of node
 * `lnum` - the line number to which this node belongs
 * `lpos` - the position in the line
 */
typedef struct node {
   enum {
      DATKIND_INT,
      DATKIND_STR
   } datkind;
   union {
      int n;
      struct {
         char *run;
         int len;
      } s;
   } dat;
   nodekind_t kind;
   int lnum, lpos;
} node_t;

tree_t *parse(
   optflg_t *of,
   optval_t *ov,
   arr_t *tokens
);

void setndn(node_t *n, int v);
void setnds(node_t *n, char *s, int l);

#endif
