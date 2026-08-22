// TODO: init opt, handle h,v (refer proofread)

#include "optprocessor.h"
#include "optprocessor.internals.h"

extern void process_opts(compile_ctx_t *cctx) {
   validate_argc(cctx->argc);

   if (!cctx->ov->src) {
      ERR("no source file given; terminating");
   }

   handle_pdtopt(cctx);
   default_stdopt(cctx);
}

static void validate_argc(int argc) {
   if (argc != 1) {
      return;
   }

   ERR("executed with no argument!\n"
      "Suggestion. type " Cbcyan "-h" Creset " or "
      Cbcyan "--help" Creset " to see a manual page");
}

static void handle_pdtopt(compile_ctx_t *cctx) {
   if (!cctx->of->pdt) {
      return;
   }

   cctx->of->w_kc = true;
   cctx->of->w_bp = true;
}

static void default_stdopt(compile_ctx_t *cctx) {
   if (cctx->of->std) {
      return;
   }

   cctx->of->std = true;
   cctx->ov->std = stdopt_spl01;
}
