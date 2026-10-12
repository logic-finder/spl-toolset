#include "config.h"
#include "config.internals.h"

extern void init_config(config_t *cfg) {
   cfg->source_files = array_create(NULL);
   cfg->job = 1;
   cfg->std = StdoptSpl01;
   cfg->lng = LngoptEn;
}

extern void destroy_config(config_t *cfg) {
   free(cfg->out);
   array_destroy(cfg->source_files);
}

extern void apply_config(config_t *cfg) {
   if (cfg->hlp) {
      print_manpage();
      exit(EXIT_SUCCESS);  /* early exit */
   }

   if (cfg->vsn) {
      print_version(&optmds[1]);
      exit(EXIT_SUCCESS);  /* early exit */
   }

   if (array_size(cfg->source_files) == 0) {
      ERR("no source file specified. Terminating.");
   }

   if (cfg->pdt) {
      cfg->w_kc = true;
      cfg->w_bp = true;
   }

   if (!cfg->out) {
      cfg->out = strdup("a.out");
   }
}

static void print_manpage(void) {
   static const char *manpage_dir = "/usr/local/share/splc/splc.1.txt";
   FILE *fp;
   int ch;

   fp = fopen(manpage_dir, "r");
   if (fp == NULL) {
      VERR("Couldn't open the manpage from %s", manpage_dir);
   }

   while ((ch = safe_fgetc(fp)) != EOF) {
      safe_fputc(stdout, ch);
   }

   safe_fclose(fp);
}

static void print_version(optmd_t *v) {
   static const char *version = "v0.0.0";
   static const char *homepage = "https://github.com/logic-finder/spl-toolset";

   if (v->seen >= 1)
      safe_vprintf(EXECNAME " %s\n", version);

   if (v->seen >= 2)
      safe_vprintf("homepage: " Cul "%s\n" Creset, homepage);
}
