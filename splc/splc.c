#include "splc.h"
#include "splc.internals.h"

int main(int argc, const char *argv[]) {
   checkif_argc1(argc, optmds, optmds_len);

   /* init */
   init_msg();
   if (!init_config(&cfg))
      goto cleanup;
   parse_argv(argc, argv, optmds, optmds_len, cfg.source_files);
   apply_config(&cfg);
   dbload(&cfg);

   if (!compile(&cfg))
      goto cleanup;

cleanup:
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
      if (usage(table, table_len, intro)) {
         exit(EXIT_SUCCESS);  /* early exit */
      }
      else {
         exit(EXIT_FAILURE);
      }
   }
}

static bool compile(config_t *cfg) {
   bool stat = false;
   compile_ctx_t cctx = {
      .cfg = cfg,
      .arena = NULL,
      .ptrace = NULL,
      .perr = "unknown fatal error",
      .phdlr = NULL
   };
   size_t total = array_size(cfg->source_files);

   if (!(cctx.arena = arena_create())) {
      goto cleanup;
   }
   if (!(cctx.ptrace = list_create(NULL))) {
      goto cleanup;
   }

   if (!arena_info(EXECNAME)) {
      return false;
   }

   for (size_t i = 0; i < total; i++) {
      cctx.sourcefile = array_peek(cfg->source_files, i);
      if (!loadfile_talkative(&cctx, cctx.sourcefile))
         goto cleanup;

      if (!lex(&cctx))
         goto cleanup;
      parse(&cctx);
      typecheck(&cctx);
      if (!ctxcheck(&cctx))
         goto cleanup;
      irgenerate(&cctx);

      if (cfg->opt)
         iroptimize(&cctx);

      if (cfg->dmp)
         irdump(&cctx);

      if (cfg->tgt)
         transpile2c(&cctx);
      else
         assemble(&cctx);

      array_destroy(cctx.toks);
      array_destroy(cctx.ls);
      arena_cleanup(cctx.arena);

      cctx.toks = NULL;
      cctx.ls = NULL;
   }

   stat = true;

cleanup:
   array_destroy(cctx.toks);
   array_destroy(cctx.ls);
   list_destroy(cctx.ptrace);
   arena_destroy(cctx.arena);

   if (!stat) {
      panic(&cctx);
      abort();
   }

   return stat;
}

static bool loadfile_talkative(compile_ctx_t *cctx, const char *filename) {
   int nchar;

   cctx->ls = loadfile(cctx->arena, filename, &cctx->lc, &cctx->wc);
   if (cctx->ls == NULL) {
      (void) fprintf(stderr, "fatal: failed to load: %s\n", cctx->sourcefile);
      return false;
   }

   if (cctx->lc == 0) {
      (void) fprintf(stderr, "fatal: empty file: %s\n", cctx->sourcefile);
      return false;  /* cleanup takes place in compile() */
   }

   if (cctx->cfg->vbs) {
      nchar = printf(
         "\n"
         ENPREFIX "loaded the source file " Cbyellow "%s" Creset,
         filename
      );
      if (nchar < 0) return false;
   }

   nchar = printf(
      " (total " Cbwhite "%zu" Creset " lines, " Cbwhite "%zu" Creset " chars)\n",
      cctx->lc, cctx->wc
   );

   return (0 <= nchar) ? true : false;
}

static void panic(compile_ctx_t *cctx) {
   (void) fprintf(stderr, "PANIC! ");

   if (cctx->phdlr) {
      (*cctx->phdlr)(cctx);
   }
   else {
      panic_default(cctx);
   }

   (void) fprintf(stderr, ". Aborting.\n");

   ptrace(cctx);
}

static void panic_default(compile_ctx_t *cctx) {
   (void) fprintf(
      stderr,
      "%s",
      cctx->perr
   );
}

static void ptrace(compile_ctx_t *cctx) {
   size_t sz = list_size(cctx->ptrace);

   ptrace_info_t *info;

   for (size_t i = 0; i < sz; i++) {
      info = list_peek(cctx->ptrace, i);
      (void) fprintf(
         stderr,
         "\tcalled at %s:%s:%zu\n",
         info->filename,
         info->funcname,
         info->ln
      );
   }
}
