// TODO: init opt, handle h,v (refer proofread)

#include "optprocessor.h"
#include "optprocessor.internals.h"

extern void process_opts(compile_ctx_t *cctx) {
   validate_argc(cctx->argc);

   if (!cctx->ov->src) {
      ERR("no source file given; terminating");
   }

   init_opt(cctx->ov);
}

static void validate_argc(int argc) {
   if (argc != 1)
      return;

   ERR("executed with no argument!\n"
      "\ttype" Cbwhite "-h" Creset " or "
      Cbwhite "--help" Creset "to see a manual page");
}

static void init_opt(optval_t *ov) {
   (void) ov;
   return;
}
