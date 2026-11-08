#include "config.h"
#include "config.internals.h"

extern bool init_config(compile_ctx_t *cctx, config_t *cfg) {
   if (!(cfg->source_files = array_create(NULL, NULL))) {
      cctx->perr = "failed to create cfg->srcs";
      goto err;
   }

   cfg->vsn = 0;
   cfg->job = 1;
   cfg->std = StdoptSpl01;
   cfg->lng = LngoptEn;

   return true;

err:
   PTRACE(cctx);
   return false;
}

extern void destroy_config(config_t *cfg) {
   free(cfg->out);
   array_destroy(cfg->source_files);
}

extern bool apply_config(compile_ctx_t *cctx, config_t *cfg) {
   // Check for -hvvv first
   if (cfg->hlp && cfg->vsn >= 3) {
      if (!easter_egg(cctx)) {
         goto err;
      }
   }

   // --help specified?
   if (cfg->hlp) {
      if (!print_manpage(cctx)) {
         goto err;
      }
   }

   // --version specified?
   if (cfg->vsn) {
      if (!print_version(cctx)) {
         goto err;
      }
   }

   // source files are not specified?
   if (array_size(cfg->source_files) == 0) {
      cctx->perr = "empty cfg->srcs";
      goto err;
   }

   // --pedantic: enables all -W flags
   if (cfg->pdt) {
      cfg->w_kc = true;
      cfg->w_bp = true;
   }

   // --output: the output file name defaults to a.out
   if (!cfg->out) {
      if (!(cfg->out = strdup("a.out"))) {
         cctx->perr = "failed to strdup a.out";
         goto err;
      }
   }

   // Normal execution path
   PH_RESET(cctx);  // Reset panic handlers which have been modified
   return true;

err:
   PTRACE(cctx);
   return false;
}

static bool print_manpage(compile_ctx_t *cctx) {
   static const char *manpage_dir = "/usr/local/share/splc/splc.1.txt";

   bool fclosed = false;

   FILE *fp;
   int ch;

   // Setup panic handlers
   cctx->phdlr = panic_print_manpage;
   cctx->pnctx = manpage_dir;

   // Open manpage
   if (!(fp = fopen(manpage_dir, "r"))) {
      cctx->perr = "failed to fopen";
      PTRACE(cctx);
      return false;
   }

   // Print manpage
   while (EOF != (ch = fgetc(fp))) {
      if (EOF == fputc(stdout, ch)) {
         cctx->perr = "failed to fputc";
         goto err;
      }
   }
   if (ferror(fp)) {
      cctx->perr = "ferror happened";
      goto err;
   }

   // Clean up
   if (fclose(fp)) {
      fclosed = true;
      cctx->perr = "failed to fclose";
      goto err;
   }

   return true;

err:
   PTRACE(cctx);
   FCLOSE_ERR_PATH(fclosed, fp);
   return false;
}

static bool print_version(compile_ctx_t *cctx) {
   static const char *version = "v0.0.0";
   static const char *homepage = "https://github.com/logic-finder/spl-toolset";

   int nchar;

   if (cfg.vsn >= 1) {
      nchar = printf(EXECNAME " %s\n", version);
      if (nchar < 0) {
         cctx->perr = "failed to print version";
         goto err;
      }
   }

   if (cfg.vsn >= 2) {
      nchar = printf("homepage: " Cul "%s\n" Creset, homepage);
      if (nchar < 0) {
         cctx->perr = "failed to print homepage";
         goto err;
      }
   }

   return true;

err:
   PTRACE(cctx);
   return false;
}

static inline bool easter_egg(compile_ctx_t *cctx) {
   int nchar = printf("블아 아즈사랑 결혼하고싶다 아즈사 너무 귀여워\n");

   if (nchar < 0) {
      cctx->perr = "failed to print easter egg";
      goto err;
   }

   return true;

err:
   PTRACE(cctx);
   return false;
}

static void panic_print_manpage(compile_ctx_t *cctx) {
   const char *filename = cctx->pnctx;

   (void) fprintf(
      stderr,
      "%s: %s",
      cctx->perr,
      filename
   );
}
