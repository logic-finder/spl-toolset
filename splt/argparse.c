#include "argparse.h"
#include "argparse.type.h"

static char *warnmsg =
   "\t* Should this be a filename, specify \"--\" first.\n"
   "\t* Type \"--\"help to see the manual page.";

extern void parse_args(const char **argv, optflg_t *of, optval_t *ov) {
   /*
    * SYNOPSIS
    *    splt [OPTIONS] [--] source
    *    splt (-h | --help)
    *    splt (-v | --version)
    *
    * OPTIONS
    *    -e, --exe
    *    -i, --keep-intermediate
    *    -d, --describe
    *    -k, --kawaii
    *    --lang=(en|ko)
    *    --ret=<name>
    */

   // initialize ov
   ov->src = NULL;

   // iterate argv and parse options.
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
         VERR("unable to recognize this option: %s.\n%s", arg, warnmsg);
      else
         parse_filenm(ov, arg);
   }
}

static void parse_endopt(optflg_t *of) {
   if (of->hlp) ERR("-h with --.");
   if (of->vsn) ERR("-v with --.");
   of->eoo = true;
}

static void parse_filenm(optval_t *ov, const char *arg) {
   // only one source file is allowed
   if (ov->src != NULL)
      ERR("a source file already specified.");
   ov->src = arg;
}

static void parse_shrtop(optflg_t *of, optval_t *_, const char *arg) {
   static const opt_t options[] = {
   //  .name   .handler
      { "e", handle_exeopt },
      { "i", handle_imdopt },
      { "d", handle_dscopt },
      { "k", handle_kwiopt },
      { "h", handle_hlpopt },
      { "v", handle_vsnopt }
   };
   static const int len = ARRLEN(options);

   // iterate the string(=arg) and parse options
   char ch;
   int p, q;

   // arg be like -eidk; read one letter at a time.
   for (p = 1; (ch = arg[p]); p++) {
      // iterate the options array
      for (q = 0; q < len; q++) {
         if (ch != options[q].name[0])
            continue;
         options[q].handler(of, _, arg);
         goto next;
      }
      VERR("unable to recognize this option: -%c.\n%s", ch, warnmsg);
      next:;
   }
}

static void parse_longop(optflg_t *of, optval_t *ov, const char *arg) {
   static const opt_t options[] = {
      { "exe", handle_exeopt },
      { "keep-intermediate", handle_imdopt },
      { "describe", handle_dscopt },
      { "kawaii", handle_kwiopt },
      { "lang", handle_lngopt },
      { "ret", handle_retopt },
      { "help", handle_hlpopt },
      { "version", handle_vsnopt }
   };
   static const int len = ARRLEN(options);

   arg += 2;  /* skips -- */

   // iterate the options array
   const char *name;

   for (int k = 0; k < len; k++) {
      name = options[k].name;
      if (strncmp(arg, name, strlen(name)))
         continue;
      options[k].handler(of, ov, arg);
      goto next;
   }
   VERR("unable to recognize this option: --%s.\n%s", arg, warnmsg);
   next:;
}

static void handle_exeopt(optflg_t *of, optval_t *_, const char *__) {
   (void) _; (void) __;  /* to suppress compiler warning message */
   if (of->exe) ERR("-e already seen.");
   if (of->hlp) ERR("-h with -e.");
   if (of->vsn) ERR("-v with -e.");
   of->exe = true;
}

static void handle_imdopt(optflg_t *of, optval_t *_, const char *__) {
   (void) _; (void) __;
   if (of->imd) ERR("-i already seen.");
   if (of->hlp) ERR("-h with -i.");
   if (of->vsn) ERR("-v with -i.");
   of->imd = true;
}

static void handle_dscopt(optflg_t *of, optval_t *_, const char *__) {
   (void) _; (void) __;
   if (of->dsc) ERR("-d already seen.");
   if (of->hlp) ERR("-h with -d.");
   if (of->vsn) ERR("-v with -d.");
   of->dsc = true;
}

static void handle_kwiopt(optflg_t *of, optval_t *_, const char *__) {
   (void) _; (void) __;
   if (of->kwi) ERR("-k already seen.");
   if (of->hlp) ERR("-h with -k.");
   if (of->vsn) ERR("-v with -k.");
   of->kwi = true;
}

static void handle_lngopt(optflg_t *of, optval_t *ov, const char *arg) {
   if (of->lng) ERR("--lang already seen.");
   if (of->hlp) ERR("-h with --lang.");
   if (of->vsn) ERR("-v with --lang.");

   if (arg[4] != '=')
      ERR("there is no '=' between --lang and its value.");

   const char *langs[] = {
      "en", "ko"
   };
   const int len = ARRLEN(langs);
   const char *val;

   for (int m = 0; m < len; m++) {
      val = arg + 5;
      if (strcmp(val, langs[m]))
         continue;
      of->lng = true;
      ov->lng = val;
      goto done;
   }
   VERR("--lang with a wrong value: %s.", val);
   done:;
}

static void handle_retopt(optflg_t *of, optval_t *ov, const char *arg) {
   if (of->ret) ERR("--ret already seen.");
   if (of->hlp) ERR("-h with --ret.");
   if (of->vsn) ERR("-v with --ret.");

   if (arg[3] != '=')
      ERR("there is no '=' between --ret and its value.");

   ov->ret = arg + 4;
}

static void handle_hlpopt(optflg_t *of, optval_t *_, const char *__) {
   (void) _; (void) __;
   if (of->exe) ERR("-e with -h.");
   if (of->imd) ERR("-i with -h.");
   if (of->dsc) ERR("-d with -h.");
   if (of->kwi) ERR("-k with -h.");
   if (of->lng) ERR("--lang with -h.");
   if (of->ret) ERR("--ret with -h.");
   if (of->hlp) ERR("-h already seen.");
   if (of->vsn) ERR("-v with -h.");
   of->hlp = true;
}

static void handle_vsnopt(optflg_t *of, optval_t *_, const char *__) {
   (void) _; (void) __;
   if (of->exe) ERR("-e with -v.");
   if (of->imd) ERR("-i with -v.");
   if (of->dsc) ERR("-d with -v.");
   if (of->kwi) ERR("-k with -v.");
   if (of->lng) ERR("--lang with -v.");
   if (of->ret) ERR("--ret with -v.");
   if (of->hlp) ERR("-h with -v.");
   if (of->vsn) ERR("-v already seen.");
   of->vsn = true;
}
