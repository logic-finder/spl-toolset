#ifndef IRGEN_H
#define IRGEN_H

#include "argparse.h"

typedef enum irtokkind {
   Irtokkind_inst,   // instruction
   Irtokkind_var,    // variable name
   Irtokkind_char,   // character index
   Irtokkind_const,  // constant
} irtokkind_t;

typedef struct irtok_inst {
   irtokkind_t kind;  /* Must be the first field */
   int val, lnum, lpos;
} irtok_inst_t;

typedef struct irtok_param {
   irtokkind_t kind;  /* Must be the first field */
   int val;
} irtok_param_t;

void generate_ir(optflg_t *of, optval_t *ov);

#endif
