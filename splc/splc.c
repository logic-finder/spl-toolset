#include "splc.h"
#include "splc.internals.h"

int main(int argc, const char *argv[]) {
   compile_ctx_t cctx;

   cctx.argc = argc;
   cctx.argv = argv;

   /* Initialization */
   init_msg();
   parse_args(&cctx);
   process_opts(&cctx);

   cctx.ls = loadfile(cctx.ov->src, &cctx.lc, &cctx.wc);
   safe_vprintf(ENPREFIX
      "loaded the source file " Cbyellow "%s" Creset
      " (total " Cbwhite "%d" Creset " lines, " Cbwhite "%d" Creset " chars)\n",
      cctx.ov->src, cctx.lc, cctx.wc);

   dbload();

   /* MAIN LOGIC */
   lex(&cctx);
   parse(&cctx);
   typecheck(&cctx);
   ctxcheck(&cctx);

   // transpile(&of, &ov);
   irgenerate(&cctx);

   if (cctx.of->opt)
      iroptimize(&cctx);

   if (cctx.of->dmp)
      irdump(&cctx);

   if (cctx.of->tgt)
      transpile2c(&cctx);
   else
      assemble(&cctx);

   /* Cleanup */
   destroy_irt(&cctx);
   destroy_pt(&cctx);
   array_destroy(cctx.toks);
   dbunload();
   array_destroy(cctx.ls);
   destroy_options(&cctx);

   return 0;
}
