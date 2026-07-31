#include "argparse.h"
#include "argparse.internals.h"

extern void parse_args(const char **argv, optflg_t *of, optval_t *ov) {
   /*
    * SYNOPSIS
    *    splc [OPTIONS] [--] <source>
    *    splc (-h | --help)
    *    splc (-v | --version)
    *
    * OPTIONS
    *    -e, --exe
    *    -i, --keep-intermediate
    *    -d, --describe
    *    -k, --kawaii
    *    --lang=(en|ko)
    *    --ret=<name>
    */

   // Initialize ov
   ov->src = NULL;

   // Iterate argv and parse options
   const char *arg;

   for (int i = 1; (arg = argv[i]); i++) {
      if (of->eoo)
         parse_filenm(ov, arg);
      else
      if (strlen(arg) > 1 && strspn(arg, "-") == 1)
         parse_shrtop(of, ov, arg);
      else
      if (strlen(arg) > 2 && strspn(arg, "-") == 2)
         parse_longop(of, ov, arg);
      else
      if (strlen(arg) == 2 && !strcmp(arg, "--"))
         parse_endopt(of);
      else
      if (arg[0] == '-')
         VERR("unable to recognize this option: %s\n%s", arg, warnmsg);
      else
         parse_filenm(ov, arg);
   }
}

static void parse_endopt(optflg_t *of) {
   if (of->hlp) ERR("-h with --");
   if (of->vsn) ERR("-v with --");
   of->eoo = true;
}

static void parse_filenm(optval_t *ov, const char *arg) {
   // Only one source file is allowed
   if (ov->src)
      ERR("a source file already specified");
   ov->src = arg;
}

static void parse_shrtop(optflg_t *of, optval_t *ov, const char *arg) {
   static const opt_t opts[] = {
   //  .name    .handler
      { "e" , handle_exeopt },
      { "i" , handle_imdopt },
      { "d" , handle_dscopt },
      { "k" , handle_kwiopt },
      { "h" , handle_hlpopt },
      { "v" , handle_vsnopt }
   };
   static const int opts_len = ARRLEN(opts);

   // Iterate the string(=arg) and parse options
   const opt_t *opt;
   char ch;
   int p, q;

   // `arg` be like -eidk; read one letter at a time
   for (p = 1; (ch = arg[p]); p++) {
      // iterate the `opts` array
      for (q = 0; q < opts_len; q++) {
         opt = opts + q;
         if (ch != opt->name[0])
            continue;
         (*opt->handler)(of, ov, arg);
         goto next;
      }
      VERR("unable to recognize this option: -%c\n%s", ch, warnmsg);
      next:;
   }
}

static void parse_longop(optflg_t *of, optval_t *ov, const char *arg) {
   static const opt_t opts[] = {
      { "exe"               , handle_exeopt },
      { "keep-intermediate" , handle_imdopt },
      { "describe"          , handle_dscopt },
      { "kawaii"            , handle_kwiopt },
      { "lang"              , handle_lngopt },
      { "ret"               , handle_retopt },
      { "help"              , handle_hlpopt },
      { "version"           , handle_vsnopt }
   };
   static const int opts_len = ARRLEN(opts);

   arg += 2;  /* skips -- */

   // Iterate the options array
   const opt_t *opt;
   const char *name;

   for (int i = 0; i < opts_len; i++) {
      opt = opts + i;
      name = opt->name;
      // some options take value, so can't use strcmp here
      if (strncmp(arg, name, strlen(name)))
         continue;
      (*opt->handler)(of, ov, arg);
      goto next;
   }
   VERR("unable to recognize this option: --%s\n%s", arg, warnmsg);
   next:;
}

static void handle_exeopt(optflg_t *of, optval_t *_, const char *__) {
   (void) _; (void) __;  /* to suppress compiler warning message */
   if (of->exe) ERR("-e already seen");
   if (of->hlp) ERR("-h with -e");
   if (of->vsn) ERR("-v with -e");
   of->exe = true;
}

static void handle_imdopt(optflg_t *of, optval_t *_, const char *__) {
   (void) _; (void) __;
   if (of->imd) ERR("-i already seen");
   if (of->hlp) ERR("-h with -i");
   if (of->vsn) ERR("-v with -i");
   of->imd = true;
}

static void handle_dscopt(optflg_t *of, optval_t *_, const char *__) {
   (void) _; (void) __;
   if (of->dsc) ERR("-d already seen");
   if (of->hlp) ERR("-h with -d");
   if (of->vsn) ERR("-v with -d");
   of->dsc = true;
}

static void handle_kwiopt(optflg_t *of, optval_t *_, const char *__) {
   (void) _; (void) __;
   if (of->kwi) ERR("-k already seen");
   if (of->hlp) ERR("-h with -k");
   if (of->vsn) ERR("-v with -k");
   of->kwi = true;
}

static void handle_lngopt(optflg_t *of, optval_t *ov, const char *arg) {
   static const int lngopt_len = 4;
   static const char *langs[] = {
      "en", "ko"
   };
   static const int len = ARRLEN(langs);

   if (of->lng) ERR("--lang already seen");
   if (of->hlp) ERR("-h with --lang");
   if (of->vsn) ERR("-v with --lang");

   if (arg[lngopt_len] != '=')
      ERR("there is no '=' between --lang and its value");

   arg += lngopt_len + 1;
   for (int m = 0; m < len; m++) {
      if (!strcmp(arg, langs[m]))
         goto done;
   }
   VERR("--lang with a wrong value: %s", arg);

   done:
      of->lng = true;
      ov->lng = arg;
}

static void handle_retopt(optflg_t *of, optval_t *ov, const char *arg) {
   static const int retopt_len = 3;

   if (of->ret) ERR("--ret already seen");
   if (of->hlp) ERR("-h with --ret");
   if (of->vsn) ERR("-v with --ret");

   if (arg[retopt_len] != '=')
      ERR("there is no '=' between --ret and its value");

   ov->ret = arg + 4;
}

static void handle_hlpopt(optflg_t *of, optval_t *_, const char *__) {
   (void) _; (void) __;
   if (of->exe) ERR("-e with -h");
   if (of->imd) ERR("-i with -h");
   if (of->dsc) ERR("-d with -h");
   if (of->kwi) ERR("-k with -h");
   if (of->lng) ERR("--lang with -h");
   if (of->ret) ERR("--ret with -h");
   if (of->hlp) ERR("-h already seen");
   if (of->vsn) ERR("-v with -h");
   of->hlp = true;
}

static void handle_vsnopt(optflg_t *of, optval_t *_, const char *__) {
   (void) _; (void) __;
   if (of->exe) ERR("-e with -v");
   if (of->imd) ERR("-i with -v");
   if (of->dsc) ERR("-d with -v");
   if (of->kwi) ERR("-k with -v");
   if (of->lng) ERR("--lang with -v");
   if (of->ret) ERR("--ret with -v");
   if (of->hlp) ERR("-h with -v");
   if (of->vsn) ERR("-v already seen");
   of->vsn = true;
}
