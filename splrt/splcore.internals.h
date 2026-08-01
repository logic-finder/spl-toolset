#ifndef SPLRT_INTERNALS_H
#define SPLRT_INTERNALS_H

#include "global.h"

typedef void iohandler_template_t(
   runtime_context_t *rctx,
   size_t charidx,
   const char *fmt
);

static stack_t **init_mem(size_t dpsz);
static void cleanup_mem(runtime_context_t *rctx);

static iohandler_template_t io_out;
static iohandler_template_t io_in;

void assert_offstage(stage_t *st, int who);
void assert_onlytwo(stage_t *st);

static void clearbuf(void);

static const char *errmsg_charidx_oob = "charidx out of bound. Terminating";

#endif
