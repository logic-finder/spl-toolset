#include "argparse.h"
#include "argparse.type.h"

/**********************
 * External Variables *
 **********************/
optflg_t of = {0};
optval_t ov;

extern void parse_args(const char **argv) {
   /*
    * SYNOPSIS
    *    spla [-n | --no-warn] [-k | --kawaii] [--] <object>
    *    spla (-h | --help)
    *    spla (-v | --version)
    */

   // Initialize ov
   ov.src = NULL;

   // Iterate argv and parse options
   const char *arg;

   for (int i = 1; (arg = argv[i]); i++) {
      if (of.eoo)
         parse_filenm(arg);
      else
      if (strlen(arg) > 1 && strspn(arg, "-") == 1)
         parse_shrtop(arg);
      else
      if (strlen(arg) > 2 && strspn(arg, "-") == 2)
         parse_longop(arg);
      else
      if (strlen(arg) == 2 && !strcmp(arg, "--"))
         parse_endopt();
      else
      if (arg[0] == '-')
         VERR("unable to recognize this option: %s", arg);
   }
}

static void parse_endopt(void) {
   if (of.hlp) ERR("--help with --");
   if (of.vsn) ERR("--version with --");
   of.eoo = true;
}

static void parse_filenm(const char *arg) {
   // Only one source file is allowed
   if (ov.src)
      ERR("an object file already specified");
   ov.src = arg;
}

static void parse_shrtop(const char *arg) {
   static const opt_t opts[] = {
      { "n" , handle_nwnopt },
      { "k" , handle_kwiopt },
      { "h" , handle_hlpopt },
      { "v" , handle_vsnopt }
   };
   static const int opts_len = ARRLEN(opts);

   // Iterate the string(=arg) ahd parse options
   const opt_t *opt;
   char ch;
   int p, q;

   // `arg` be like -nk; read one letter at a time
   for (p = 1; (ch = arg[p]); p++) {
      // iterate the `opts` array
      for (q = 0; q < opts_len; q++) {
         opt = opts + q;
         if (ch != opt->name[0])
            continue;
         (*opt->handler)(arg);
         goto next;
      }
      VERR("unable to recognize this option: -%c", ch);
      next:;
   }
}

static void parse_longop(const char *arg) {
   static const opt_t opts[] = {
      { "no-warn" , handle_nwnopt },
      { "kawaii"  , handle_kwiopt },
      { "help"    , handle_hlpopt },
      { "version" , handle_vsnopt }
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
      (*opt->handler)(arg);
      goto next;
   }
   VERR("unable to recognize this option: --%s", arg);
   next:;
}

static void handle_nwnopt(const char *arg) {
   (void) arg;
   if (of.nwn) ERR("--no-warn already seen");
   if (of.hlp) ERR("--help with --no-warn");
   if (of.vsn) ERR("--version with --no-warn");
   of.nwn = true;
}

static void handle_kwiopt(const char *arg) {
   (void) arg;
   if (of.kwi) ERR("--kawaii already seen");
   if (of.hlp) ERR("--help with --kawaii");
   if (of.vsn) ERR("--version with --kawaii");
   of.kwi = true;
}

static void handle_hlpopt(const char *arg) {
   (void) arg;
   if (of.nwn) ERR("--no-warn with --help");
   if (of.kwi) ERR("--kawaii with --help");
   if (of.hlp) ERR("--help already seen");
   if (of.vsn) ERR("--version with --help");
   of.hlp = true;
}

static void handle_vsnopt(const char *arg) {
   (void) arg;
   if (of.nwn) ERR("--no-warn with --help");
   if (of.kwi) ERR("--kawaii with --help");
   if (of.hlp) ERR("--help with --version");
   if (of.vsn) ERR("--version already seen");
   of.vsn = true;
}
