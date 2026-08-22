#include "optprocessor.h"
#include "optprocessor.internals.h"

extern void process_opts(compile_ctx_t *cctx) {
   validate_argc(cctx->argc);

   handle_hlpopt(cctx);
   handle_vsnopt(cctx);

   if (!cctx->ov->src) {
      ERR("no source file given; terminating");
   }

   handle_pdtopt(cctx);
   default_stdopt(cctx);
   default_outopt(cctx);
}

static void validate_argc(int argc) {
   if (argc != 1) {
      return;
   }

   ERR("executed with no argument!\n"
      "Suggestion. type " Cbcyan "-h" Creset " or "
      Cbcyan "--help" Creset " to see a manual page");
}

static void handle_hlpopt(compile_ctx_t *cctx) {
   static const char *txtdir = "/usr/local/share/splc/splc.1.txt";

   FILE *fp;
   int ch;

   if (!cctx->of->hlp) {
      return;
   }

   /* Prints the manual file */
   fp = safe_fopen(txtdir, "r");

   while ((ch = safe_fgetc(fp)) != EOF) {
      safe_fputc(stdout, ch);
   }

   safe_fclose(fp);

   exit(EXIT_SUCCESS);
}

static void handle_vsnopt(compile_ctx_t *cctx) {
   static const char *version = "v0.0.0";

   if (!cctx->of->vsn) {
      return;
   }

   safe_vprintf("splc %s\n", version);

   exit(EXIT_SUCCESS);
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

   cctx->ov->std = stdopt_spl01;
}

static void default_outopt(compile_ctx_t *cctx) {
   static const char *default_output = "a.out";

   if (cctx->of->out) {
      return;
   }

   cctx->ov->out = default_output;
}
