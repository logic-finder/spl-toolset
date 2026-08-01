#ifndef SPLRT_H
#define SPLRT_H

/************
 * INCLUDES *
 ************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "fatal.h"
#include "common.h"
#include "wrappers.h"
#include "colorcode.h"
#include "stack.adt.h"
#include "stage.adt.h"

/************
 * TYPEDEFS *
 ************/
typedef struct {
   bool cond;
   spl_int_t *dp;  /* int [] */
   size_t dpsz;
   stack_t **mem;  /* stack_t * [] */
   stage_t *st;
   size_t
      t,  /* teller */
      h;  /* hearer */
} runtime_context_t;

typedef spl_int_t operator_t(spl_int_t v);
typedef void iohandler_t(runtime_context_t *rctx, size_t charidx);

/**************
 * PROTOTYPES *
 **************/
runtime_context_t *init_runtime(size_t dpsz);

operator_t op_sqrt;
operator_t op_squr;
operator_t op_cube;
operator_t op_fact;

iohandler_t io_inn;
iohandler_t io_inc;
iohandler_t io_outn;
iohandler_t io_outc;

#endif
