#ifndef IRGEN_TYPE_H
#define IRGEN_TYPE_H

#include <stdlib.h>
#include <stdbool.h>
#include "global.h"
#include "wrapper.h"
#include "strutil.h"
#include "arr.adt.h"
#include "argparse.h"
#include "tree.adt.h"
#include "ir.common.h"

#define IR_EXTENSION ".splir"

static void gen_dpsz(void);
static void write_irfile(void);
static void mark_inst(ir_inst_t instkind, int lnum, int lpos);
static void mark_param(irtokkind_t kind, int val);

#endif
