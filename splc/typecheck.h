#ifndef TYPECHECK_H
#define TYPECHECK_H

#include "splc.h"
#include "argparse.h"
#include "tree.adt.h"

void typecheck(compile_ctx_t *cctx);
unsigned int interpret_romnum(const char *romnum);

#endif
