#include "splc.h"
#include "splc.internals.h"

int main(int argc, const char *argv[]) {
   checkif_argc1(argc, optmds, optmds_len);

   /* init */
   init_msg();
   init_config(&cfg);
   parse_argv(argc, argv, optmds, optmds_len, cfg.source_files);
   apply_config(&cfg);
   dbload(&cfg);

   compile(&cfg);

   /* cleanup */
   dbunload();
   destroy_config(&cfg);

   return 0;
}

static void checkif_argc1(
   int argc,
   optmd_t table[],
   size_t table_len
) {
   static const char *intro =
      "NAME\n"
      "\t" Cbwhite EXECNAME Creset
      " - the Shakespeare Programming Language compiler\n"
      "\n"
      "USAGE\n"
      "\t" EXECNAME " [OPTIONS] [--] files...\n"
      "\t" EXECNAME " (-h | --help)\n"
      "\t" EXECNAME " (-v | -vv | --version)";

   if (argc == 1) {
      usage(table, table_len, intro);
      exit(EXIT_SUCCESS);  /* early exit */
   }
}

static void compile(config_t *cfg) {
   compile_ctx_t cctx = {
      .cfg = cfg,
      .arena = arena_create()
   };
   size_t total = array_size(cfg->source_files);

   arena_info(EXECNAME);

   for (size_t i = 0; i < total; i++) {
      cctx.sourcefile = array_peek(cfg->source_files, i);
      loadfile_talkative(&cctx, cctx.sourcefile);

      lex(&cctx);
      parse(&cctx);
      typecheck(&cctx);
      ctxcheck(&cctx);
      irgenerate(&cctx);

      if (cfg->opt)
         iroptimize(&cctx);

      if (cfg->dmp)
         irdump(&cctx);

      if (cfg->tgt)
         transpile2c(&cctx);
      else
         assemble(&cctx);

      /* cleanup */
      arena_cleanup(cctx.arena);
      array_destroy(cctx.toks);
      array_destroy(cctx.ls);
   }
   arena_destroy(cctx.arena);
}

static void loadfile_talkative(compile_ctx_t *cctx, const char *filename) {
   cctx->ls = loadfile(filename, &cctx->lc, &cctx->wc);
   if (!cctx->cfg->vbs)
      return;
   safe_vprintf(
      "\n"
      ENPREFIX "loaded the source file " Cbyellow "%s" Creset,
      filename
   );
   safe_vprintf(
      " (total " Cbwhite "%zu" Creset " lines, " Cbwhite "%zu" Creset " chars)\n",
      cctx->lc, cctx->wc
   );
}
