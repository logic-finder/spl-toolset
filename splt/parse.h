#ifndef PARSE_H
#define PARSE_H

#include "tree.h"
#include "loadfile.h"
#include "argparse.h"

void parse(
   optflg_t *of, optval_t *ov,
   line_t *arr, int len,
   tree_t *pt
);

#endif
