#ifndef PARSE_H
#define PARSE_H

#include "tree.h"
#include "list.adt.h"
#include "argparse.h"

tree_t *parse(
   optflg_t *of,
   optval_t *ov,
   list_t *tokens
);

#endif
