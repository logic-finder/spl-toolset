#ifndef SPLRT_INTERNALS_H
#define SPLRT_INTERNALS_H

/************
 * INCLUDES *
 ************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>

#include "fatal.h"
#include "common.h"
#include "wrappers.h"
#include "colorcode.h"
#include "stack.adt.h"
#include "stage.adt.h"

typedef void iohandler_template_t(
   rt_ctx_t *rctx,
   const char *fmt
);

static stack_t **init_mem(size_t dpsz);
static void cleanup_mem(rt_ctx_t *rctx);

static iohandler_template_t io_out;
static iohandler_template_t io_in;

static void assert_onlytwo(rt_ctx_t *rctx);

static void clearbuf(void);

// fixme: no need?
// static const char *errmsg_charidx_oob = "charidx out of bound. Terminating";

#endif
