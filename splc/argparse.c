#include "argparse.h"
#include "argparse.internals.h"

extern void parse_args(compile_ctx_t *cctx) {
   /* SYNOPSIS
         splc [OPTIONS] [--] <source>
         splc (-h | --help)
         splc (-v | --version)

      OPTIONS
         -e, --exe
         -i, --keep-intermediate // replaced by -S
         -d, --describe // verbose?
         -k, --kawaii
         -h,
         -v
         -g,
         -O,
         -S,
         --,
         --target=c
         --lang=(en|ko)
         --o=<name>
         --dry-run
         --pedantic
         --std
         --W  */

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
      else
         parse_filenm(cctx->ov, arg);
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
      { "e" , handle_exeopt },
      { "i" , handle_imdopt },
      { "d" , handle_dscopt },
      { "k" , handle_kwiopt },
      { "g" , handle_dbgopt },
      { "O" , handle_optopt },
      { "S" , handle_dmpopt },
      { "h" , handle_hlpopt },
      { "v" , handle_vsnopt }
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
      { "exe"               , handle_exeopt },
      { "keep-intermediate" , handle_imdopt },
      { "describe"          , handle_dscopt },
      { "kawaii"            , handle_kwiopt },
      { "target"            , handle_tgtopt },
      { "lang"              , handle_lngopt },
      { "o"                 , handle_outopt },
      { "dry-run"           , handle_drnopt },
      { "pedantic"          , handle_pdtopt },
      { "W"                 , handle_wrnopt },
      { "std"               , handle_stdopt },
      { "help"              , handle_hlpopt },
      { "version"           , handle_vsnopt }
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

static void handle_exeopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;  /* to suppress compiler warning message */
   if (of->exe) ERR("-e already seen");
   if (of->hlp) ERR("-h with -e");
   if (of->vsn) ERR("-v with -e");
   of->exe = true;
}

static void handle_imdopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;
   if (of->imd) ERR("-i already seen");
   if (of->hlp) ERR("-h with -i");
   if (of->vsn) ERR("-v with -i");
   of->imd = true;
}

static void handle_dscopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;
   if (of->dsc) ERR("-d already seen");
   if (of->hlp) ERR("-h with -d");
   if (of->vsn) ERR("-v with -d");
   of->dsc = true;
}

static void handle_kwiopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;
   if (of->kwi) ERR("-k already seen");
   if (of->hlp) ERR("-h with -k");
   if (of->vsn) ERR("-v with -k");
   of->kwi = true;
}

static void handle_lngopt(optflg_t *of, optval_t *ov, const char *arg) {
   static const size_t lngopt_len = 4;  /* strlen("lang") = 4 */
   static const char *langs[] = {
      "en", "ko"
   };
   static const size_t len = ARRLEN(langs);

   size_t i;

   if (of->lng) ERR("--lang already seen");
   if (of->hlp) ERR("-h with --lang");
   if (of->vsn) ERR("-v with --lang");

   if (arg[lngopt_len] != '=')
      ERR("there is no '=' between --lang and its value");

   arg += lngopt_len + 1;
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

static void handle_outopt(optflg_t *of, optval_t *ov, const char *arg) {
   if (of->out) ERR("--o already seen");
   if (of->hlp) ERR("-h with --ret");
   if (of->vsn) ERR("-v with --ret");

   arg += strlen(outopt);

   if (arg[0] != '=') {
      VERR("there is no '=' after '--%s'", outopt);
   }

   arg++;  /* skips '=' */

   of->out = true;
   ov->out = arg;
}

static void handle_optopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   of->opt = true;
}

static void handle_dbgopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   of->dbg = true;
}

static void handle_dmpopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   of->dmp = true;
}

static void handle_tgtopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   of->tgt = true;
}

static void handle_drnopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   of->drn = true;
}

static void handle_pdtopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   of->pdt = true;
}

static void handle_wrnopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   // TODO: arg check

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

static void handle_stdopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   // TODO: arg check

   arg += strlen("std");

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

static void handle_hlpopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   if (of->exe) ERR("-e with -h");
   if (of->imd) ERR("-i with -h");
   if (of->dsc) ERR("-d with -h");
   if (of->kwi) ERR("-k with -h");
   if (of->lng) ERR("--lang with -h");
   if (of->out) ERR("--ret with -h");
   if (of->hlp) ERR("-h already seen");
   if (of->vsn) ERR("-v with -h");

   of->hlp = true;
}

static void handle_vsnopt(optflg_t *of, optval_t *ov, const char *arg) {
   (void) ov, (void) arg;

   if (of->exe) ERR("-e with -v");
   if (of->imd) ERR("-i with -v");
   if (of->dsc) ERR("-d with -v");
   if (of->kwi) ERR("-k with -v");
   if (of->lng) ERR("--lang with -v");
   if (of->out) ERR("--ret with -v");
   if (of->hlp) ERR("-h with -v");
   if (of->vsn) ERR("-v already seen");

   of->vsn = true;
}
