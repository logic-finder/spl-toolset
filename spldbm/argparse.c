#include "argparse.h"
#include "argparse.internals.h"

extern void parse_args(const char **argv) {
   // TODO: support --basedir=... option
   /*
    * SYNOPSIS
    *    spldbm --make=<name>,<adj>,<noun>,<comp> [-k | --kawaii]
    *    spldbm --archive=<spldb> [-k | --kawaii]
    *    spldbm --restore=<spldb> [-k | --kawaii]
    *    spldbm (-h | --help)
    *    spldbm (-v | --version)
    */
   const char *arg;

   for (int i = 1; (arg = argv[i]); i++) {
      if (strlen(arg) > 1 && strspn(arg, "-") == 1)
         parse_shrtop(arg);
      else
      if (strlen(arg) > 2 && strspn(arg, "-") == 2)
         parse_longop(arg);
      else
         VERR("unable to recognize this option: %s", arg);
   }
}

static void parse_shrtop(const char *arg) {
   static const opt_t opts[] = {
   //  .name    .handler
      { "h" , handle_hlpopt },
      { "v" , handle_vsnopt }
   };
   static const int opts_len = ARRLEN(opts);

   const opt_t *opt;
   char ch;
   int p, q;

   for (p = 1; (ch = arg[p]); p++) {
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
      { "make"    , handle_makopt },
      { "archive" , handle_arcopt },
      { "restore" , handle_resopt },
      { "kawaii"  , handle_kwiopt },
      { "help"    , handle_hlpopt },
      { "version" , handle_vsnopt }
   };
   static const int opts_len = ARRLEN(opts);

   arg += 2;  /* skips -- */

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

static void handle_makopt(const char *arg) {
   static const int makopt_len = 4;

   if (of.mak) ERR("--make already seen");
   if (of.arc) ERR("--archive with --make");
   if (of.res) ERR("--restore with --make");
   if (of.hlp) ERR("--help with --make");
   if (of.vsn) ERR("--version with --make");

   if (arg[makopt_len] != '=')
      ERR("there is no '=' between --make and its value");

   char **components;
   size_t clen;

   arg += makopt_len + 1;
   components = split(arg, ",", &clen);
   if (clen != 4)
      VERR("--make needs 4 comma-separated filenames;"
           "currently %d given", clen);
   for (size_t i = 0; i < clen; i++)
      if (components[i][0] == '\0')
         VERR("field %d of --make is empty", i);

   ov.mak = components;
   of.mak = true;
}

static void handle_arcopt(const char *arg) {
   static const int arcopt_len = 7;

   if (of.mak) ERR("--make with --archive");
   if (of.arc) ERR("--archive already seen");
   if (of.res) ERR("--restore with --archive");
   if (of.hlp) ERR("--help with --archive");
   if (of.vsn) ERR("--version with --archive");

   if (arg[arcopt_len] != '=')
      ERR("there is no '=' between --archive and its value");

   arg += arcopt_len + 1;
   ov.arc = arg;
   of.arc = true;
}

static void handle_resopt(const char *arg) {
   static const int resopt_len = 7;

   if (of.mak) ERR("--make with --restore");
   if (of.arc) ERR("--archive with --restore");
   if (of.res) ERR("--restore already seen");
   if (of.hlp) ERR("--help with --restore");
   if (of.vsn) ERR("--version with --restore");

   if (arg[resopt_len] != '=')
      ERR("there is no '=' between --restore and its value");

   arg += resopt_len + 1;
   ov.res = arg;
   of.res = true;
}

static void handle_kwiopt(const char *arg) {
   (void) ov; (void) arg;
   if (of.kwi) ERR("--kawaii already seen");
   if (of.hlp) ERR("--help with --kawaii");
   if (of.vsn) ERR("--version with --kawaii");
   of.kwi = true;
}

static void handle_hlpopt(const char *arg) {
   (void) ov; (void) arg;
   if (of.mak) ERR("--make with --help");
   if (of.arc) ERR("--archive with --help");
   if (of.res) ERR("--restore with --help");
   if (of.kwi) ERR("--kawaii with --help");
   if (of.hlp) ERR("--help already seen");
   if (of.vsn) ERR("--version with --help");
   of.hlp = true;
}

static void handle_vsnopt(const char *arg) {
   (void) ov; (void) arg;
   if (of.mak) ERR("--make with --version");
   if (of.arc) ERR("--archive with --version");
   if (of.res) ERR("--restore with --version");
   if (of.kwi) ERR("--kawaii with --version");
   if (of.hlp) ERR("--help with --version");
   if (of.vsn) ERR("--version already seen");
   of.vsn = true;
}
