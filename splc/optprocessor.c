// TODO: init opt, handle h,v (refer proofread)

#include "optprocessor.h"
#include "optprocessor.internals.h"

extern void process_opts(compile_ctx_t *cctx) {
   validate_argc(cctx->argc);

   if (!cctx->ov->src) {
      ERR("no source file given; terminating");
   }

   activate_pdtopt(cctx->of);
}

static void validate_argc(int argc) {
   if (argc != 1)
      return;

   ERR("executed with no argument!\n"
      "\ttype" Cbwhite "-h" Creset " or "
      Cbwhite "--help" Creset "to see a manual page");
}

static void activate_pdtopt(optflg_t *of) {
   if (!of->pdt) {
      return;
   }

   of->w_kc = true;
   of->w_bp = true;
}
