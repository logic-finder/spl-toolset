#ifndef PARSE_H
#define PARSE_H

#include "arr.adt.h"
#include "tree.adt.h"
#include "argparse.h"

#define NODE_TAG_LEN 31
#
#define TREE_CHDAT(t,i) ((node_t *) tree_chdat((t),(i)))

typedef struct node {
   char *run;
   int len;
   char tag[NODE_TAG_LEN + 1];
   int lnum, lpos;
} node_t;

tree_t *parse(
   optflg_t *of,
   optval_t *ov,
   arr_t *tokens
);

#endif
