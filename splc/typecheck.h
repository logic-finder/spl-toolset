#ifndef TYPECHECK_H
#define TYPECHECK_H

#include "argparse.h"
#include "tree.adt.h"

void typecheck(optflg_t *of, optval_t *ov);
unsigned int interpret_romnum(const char *romnum);

#endif
