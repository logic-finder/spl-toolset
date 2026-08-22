#include "argparse.h"
#include "argparse.internals.h"

extern void parse_args(compile_ctx_t *cctx) {
   /* SYNOPSIS
         splc [OPTIONS] [--] <source>
         splc (-h | --help)
         splc (-v | --version)

      OPTIONS
         -h, --help
         -g, --debug
         -k, --kawaii
         -v, --version
         -S, --dump
         -O, --optimize
         --W
         --asm
         --disasm
         --direct
         --verbose
         --dry-run
         --pedantic
         --o=<name>
         --std=(spl01|cor27)
         --lang=(en|ko)
         --target=<lang> */

   const char *arg;

   cctx->of = safe_calloc(1, sizeof *cctx->of);  /* {0} */
   cctx->ov = safe_malloc(sizeof *cctx->ov);

   cctx->of->eoo = false;
   cctx->ov->src = NULL;

   /* Iterates over argv and parses options */
   for (size_t i = 1; (arg = cctx->argv[i]); i++) {
      if (cctx->of->eoo)
         parse_filenm(cctx->ov, arg);
      else
      if (strlen(arg) > 1 && strspn(arg, "-") == 1)
         parse_shrtop(cctx->of, cctx->ov, arg);
      else
      if (strlen(arg) > 2 && strspn(arg, "-") == 2)
         parse_longop(cctx->of, cctx->ov, arg);
      else
      if (strlen(arg) == 2 && !strcmp(arg, "--"))
         parse_endopt(cctx->of);
      else
      if (arg[0] == '-')
         VERR("unable to recognize this option: %s\n%s", arg, warnmsg);
      else {
         parse_filenm(cctx->ov, arg);
      }
   }
}

extern void destroy_options(compile_ctx_t *cctx) {
   free(cctx->of);
   free(cctx->ov);
}

static void parse_endopt(optflg_t *of) {
   if (of->hlp) ERR("-h with --");
   if (of->vsn) ERR("-v with --");
   of->eoo = true;
}

static void parse_filenm(optval_t *ov, const char *arg) {
   /* Only one source file is allowed */
   if (ov->src)
      ERR("a source file already specified");
   ov->src = arg;
}

static void parse_shrtop(optflg_t *of, optval_t *ov, const char *arg) {
   static const opt_t opts[] = {
   /*  .name    .handler   */
      { "h" , handle_hlpopt },
      { "g" , handle_dbgopt },
      { "k" , handle_kwiopt },
      { "v" , handle_vsnopt },
      { "S" , handle_dmpopt },
      { "O" , handle_optopt }
   };
   static const size_t opts_len = ARRLEN(opts);

   /* Iterates over arg and parse options */
   const opt_t *opt;
   char ch;
   size_t q;

   /* arg would be like -eidk; reads one letter at a time */
   for (size_t p = 1; (ch = arg[p]); p++) {
      /* iterates opts */
      for (q = 0; q < opts_len; q++) {
         opt = opts + q;
         if (ch != opt->name[0])
            continue;
         (*opt->handler)(of, ov, arg);
         break;
      }
      if (q == opts_len) {
         VERR("unable to recognize this option: -%c\n%s", ch, warnmsg);
      }
   }
}

static void parse_longop(optflg_t *of, optval_t *ov, const char *arg) {
   static const opt_t opts[] = {
      { "help"     , handle_hlpopt },
      { "debug"    , handle_dbgopt },
      { "kawaii"   , handle_kwiopt },
      { "version"  , handle_vsnopt },
      { "dump"     , handle_dmpopt },
      { "optimize" , handle_optopt },
      { "direct"   , handle_drtopt },
      { "verbose"  , handle_vbsopt },
      { "dry-run"  , handle_drnopt },
      { "o"        , handle_outopt },
      { "std"      , handle_stdopt },
      { "lang"     , handle_lngopt },
      { "target"   , handle_tgtopt },
      { "asm"      , handle_asmblyopt },
      { "disasm"   , handle_disasmopt },
      { "pedantic" , handle_pdtopt },
      { "W"        , handle_wrnopt }
   };
   static const size_t opts_len = ARRLEN(opts);

   const opt_t *opt;
   const char *name;
   size_t i;

   arg += 2;  /* skips -- */

   /* Iterates over opts */
   for (i = 0; i < opts_len; i++) {
      opt = opts + i;
      name = opt->name;
      /* some options take value, so can't use strcmp here */
      if (strncmp(arg, name, strlen(name)))
         continue;
      (*opt->handler)(of, ov, arg);
      break;
   }
   if (i == opts_len) {
      VERR("unable to recognize this option: --%s\n%s", arg, warnmsg);
   }
}

static void handle_hlpopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   of->hlp = true;
}

static void handle_dbgopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   of->dbg = true;
}

static void handle_kwiopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   of->kwi = true;
}

static void handle_vsnopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   of->vsn = true;
}

static void handle_dmpopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   of->dmp = true;
}

static void handle_optopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   of->opt = true;
}

static void handle_drtopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   of->drt = true;
}

static void handle_vbsopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   of->vbs = true;
}

static void handle_drnopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   of->drn = true;
}

static void handle_outopt(optflg_t *of, optval_t *ov, const char *arg) {
   static const char *outopt = "o";

   arg += strlen(outopt);

   if (arg[0] != '=') {
      VERR("there is no '=' after '--%s'", outopt);
   }

   arg++;  /* skips '=' */

   of->out = true;
   ov->out = arg;
}

static void handle_stdopt(optflg_t *of, optval_t *ov, const char *arg) {
   static const char *stdopt = "std";

   arg += strlen(stdopt);

   if (arg[0] != '=') {
      VERR("there is no '=' after '--%s'", stdopt);
   }

   arg++;  /* skips '=' */

   if (!strcmp(arg, stdopt_spl01) || !strcmp(arg, stdopt_cor27)) {
      of->std = true;
      ov->std = arg;
   }
   else {
      VERR("--std with a wrong value: %s", arg);
   }
}

static void handle_lngopt(optflg_t *of, optval_t *ov, const char *arg) {
   static const char *lngopt = "lang";
   static const char *langs[] = {
      "en", "ko"
   };
   static const size_t len = ARRLEN(langs);

   size_t i;

   arg += strlen(lngopt);

   if (arg[0] != '=') {
      VERR("there is no '=' after --%s", lngopt);
   }

   arg++;  /* skips '=' */

   for (i = 0; i < len; i++) {
      if (!strcmp(arg, langs[i])) {
         break;
      }
   }
   if (i == len) {
      VERR("--lang with a wrong value: %s", arg);
   }

   of->lng = true;
   ov->lng = arg;
}

static void handle_tgtopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   of->tgt = true;
}

static void handle_asmblyopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   of->asmbly = true;
}

static void handle_disasmopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   of->disasm = true;
}

static void handle_pdtopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   of->pdt = true;
}

static void handle_wrnopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   arg++;  /* skips 'W' */

   if (!strcmp(arg, "keyword-case"))
      of->w_kc = true;
   else
   if (!strcmp(arg, "be-predicate"))
      of->w_bp = true;
   else {
      VERR("--W with a wrong value: %s", arg);
   }
}
