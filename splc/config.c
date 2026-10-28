#include "config.h"
#include "config.internals.h"

extern bool init_config(config_t *cfg) {
   if (!(cfg->source_files = array_create(NULL, NULL))) {
      return false;
   }
   cfg->vsn = 0;
   cfg->job = 1;
   cfg->std = StdoptSpl01;
   cfg->lng = LngoptEn;

   return true;
}

extern void destroy_config(config_t *cfg) {
   free(cfg->out);
   array_destroy(cfg->source_files);
}

extern void apply_config(config_t *cfg) {
   if (cfg->hlp && cfg->vsn >= 3) {
      easter_egg();
      exit(EXIT_SUCCESS);  // fixme: exit대신 에러코드 리턴해서 main에서 받아쓰기
   }

   if (cfg->hlp) {
      print_manpage();
      exit(EXIT_SUCCESS);  /* early exit */
   }

   if (cfg->vsn) {
      print_version();
      exit(EXIT_SUCCESS);  /* early exit */
   }

   if (array_size(cfg->source_files) == 0) {
      ERR("no source file specified. Terminating.");
   }

   /* --pedantic: enables all -W flags */
   if (cfg->pdt) {
      cfg->w_kc = true;
      cfg->w_bp = true;
   }

   /* --output: the output file name defaults to a.out */
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

static void print_version(void) {
   static const char *version = "v0.0.0";
   static const char *homepage = "https://github.com/logic-finder/spl-toolset";

   if (cfg.vsn >= 1)
      safe_vprintf(EXECNAME " %s\n", version);

   if (cfg.vsn >= 2)
      safe_vprintf("homepage: " Cul "%s\n" Creset, homepage);
}

static inline void easter_egg(void) {
   safe_vprintf("블아 아즈사랑 결혼하고싶다 아즈사 너무 귀여워\n");
}
