#include "argp.h"
#include "argp.internals.h"

/*
Improvement Ideas
   - argument taking optional value
      -O => uses its default value
      -O3 => uses 3 as its value
*/

extern void _parse_args(compile_ctx_t *cctx) {
   argp_ctx_t actx = {0};

   actx.argc = cctx->argc;
   actx.argv = cctx->argv;
   actx.toks = array_create(destruct_argtok);
   actx.idx = 0;
   actx.eoe  = false;
   actx.errcnt = 0;

   init_config();

   validate_schemas();
   tokenize_argv(&actx);
   parse_argtoks(&actx);

   if (actx.errcnt > 0) {
      exit(EXIT_FAILURE);
   }

showtoks(&actx, true);

   array_destroy(actx.toks);
}

extern void destroy_config(void) {
   free(cfg.out);
   array_destroy(cfg.source_files);
}

static void init_config(void) {
   cfg.source_files = array_create(NULL);
   cfg.job = 1;
   cfg.out = NULL;
   cfg.std = StdoptSpl01;
   cfg.lng = LngoptEn;
}

static void validate_schemas(void) {
   size_t i, j;
   optmd_t *subject, *against;
   size_t count_dup;

   count_dup = 0;

   /* short flags */
   for (i = 0; i < optmds_len; i++) {
      subject = &optmds[i];
      if (subject->sname == '\0')
         continue;
      for (j = i + 1; j < optmds_len; j++) {
         against = &optmds[j];
         if (against->sname == '\0')
            continue;
         if (subject->sname == against->sname) {
            safe_vprintf("md[%zu] has the same short name with md[%zu].\n", i, j);
            count_dup++;
         }
      }
   }

   /* long flags */
   for (i = 0; i < optmds_len; i++) {
      subject = &optmds[i];
      if (subject->lname == NULL)
         continue;
      for (j = i + 1; j < optmds_len; j++) {
         against = &optmds[j];
         if (against->lname == NULL)
            continue;
         if (!strcmp(subject->lname, against->lname)) {
            safe_vprintf("md[%zu] has the same short name with md[%zu].\n", i, j);
            count_dup++;
         }
      }
   }

   if (count_dup == 0)
      return;

   safe_fputs(stdout, "Currently the program has a problematic option table. Terminating.\n");
   exit(EXIT_FAILURE);
}

static void tokenize_argv(argp_ctx_t *actx) {
   /* Note: begins from 1 to skip the program name */
   for (int i = 1; i < actx->argc; i++) {
      actx->arg = actx->argv[i];

      if (actx->eoe) {
         populate_as_posarg(actx);
         continue;
      }

      if (!strcmp(actx->arg, "--")) {
         actx->eoe = true;
         populate_as_dashdash(actx);
      }
      else
      if (!strncmp(actx->arg, "--", 2)) {
         populate_as_longflag(actx);
      }
      else
      if (actx->arg[0] == '-' && actx->arg[1] != '\0') {
         populate_as_shortflag(actx);
      }
      else {
         populate_as_posarg(actx);
      }
   }

   actx->toks_len = array_size(actx->toks);
}

static void parse_argtoks(argp_ctx_t *actx) {
   argtok_t *tok;

   while (actx->idx < actx->toks_len) {
      tok = gettok(actx);

      /* end of option? */
      if (tok->kind == ArgtokOptionEnd) {
         break;
      }

      /* positional argument? */
      if (tok->kind == ArgtokPosArg) {
         handle_posarg(actx);
         continue;
      }

      /* Since it's neither -- nor posarg, it's either
         a short or long flag, meaning tok->md must exist */

      /* specified multiple times? */
      if (tok->md->seen++ && tok->md->unique == true) {
         issue_err(actx,
            "%s: already specified %zu times", tok->field, tok->md->seen);
         continue;
      }

      switch (tok->kind) {
         /* to be accepted: "-j" | "-j4" | "-j 4" */
         case ArgtokShortFlag : preprocess_shortflag(actx); break;
         /* to be accepted: "--job" | "--job=4" | "--job 4" */
         case ArgtokLongFlag  : preprocess_longflag (actx); break;
         /* control never reaches default */
         default: ;
      }

      /* since actx->tok can have been set to lookahead (i.e. posarg)
         in preprocess(), need to reset its value to tok (i.e. option) */
      actx->tok = tok;
      (*tok->md->handle)(actx);
      /* Note that actx->idx is still the index of lookahead */
   }

   /* Collects the remanining positional arguments as filenames */
   while (actx->idx < actx->toks_len) {
      gettok(actx);
      handle_posarg(actx);
   }
}

static void populate_as_dashdash(argp_ctx_t *actx) {
   argtok_t tok = {
      .kind = ArgtokOptionEnd,
      .field = NULL,
      .value = NULL,
      .md = NULL,
      .has_eq = false
   };

   array_append(actx->toks, &tok, sizeof tok);
}

static void populate_as_longflag(argp_ctx_t *actx) {
   argtok_t tok = {
      .kind = ArgtokLongFlag,
      .field = NULL,
      .value = NULL,
      .md = NULL,
      .has_eq = false
   };
   size_t eq_pos, arg_len;

   /*   01234567890123
      --target=haskell
        ^      ^
        field  value */

   actx->arg += 2;  /* skips -- */
   eq_pos = strcspn(actx->arg, "=");
   arg_len = strlen(actx->arg);

   tok.field = strndup(actx->arg, eq_pos);
   tok.md = query_by_longname(tok.field);

   if (tok.md == NULL) {
      issue_err(actx, "--%s: unrecognized long flag", tok.field);
      free(tok.field);
      return;
   }

   if (eq_pos < arg_len) {
      tok.has_eq = true;
      /* = exists but no value given */
      if (arg_len - eq_pos > 1) {
         tok.value = strdup(&actx->arg[eq_pos + 1]);
      }
   }

   array_append(actx->toks, &tok, sizeof tok);
}

static void populate_as_shortflag(argp_ctx_t *actx) {
   argtok_t tok = {
      .kind = ArgtokShortFlag,
      .field = NULL,
      .value = NULL,
      .md = NULL,
      .has_eq = false
   };
   char flag;
   size_t arg_len;

   actx->arg++;  /* skips - */
   arg_len = strlen(actx->arg);

   for (
      /* empty */;
      (flag = actx->arg[0]) != '\0';
      actx->arg++, arg_len--
   ) {
      tok.md = query_by_shortname(flag);

      if (tok.md == NULL) {
         issue_err(actx, "-%c: unrecognized short flag", flag);
         continue;
      }

      tok.field = strndup(actx->arg, 1);

      if (tok.md->reqv && arg_len > 1) {
         tok.value = strdup(&actx->arg[1]);
      }

      array_append(actx->toks, &tok, sizeof tok);

      if (tok.md->reqv) {
         return;
      }
   }
}

static void populate_as_posarg(argp_ctx_t *actx) {
   argtok_t tok = {
      .kind = ArgtokPosArg,
      .field = strdup(actx->arg),
      .value = NULL,
      .md = NULL,
      .has_eq = false
   };

   array_append(actx->toks, &tok, sizeof tok);
}

static optmd_t *query_by_shortname(char flag) {
   optmd_t *md;

   for (size_t i = 0; i < optmds_len; i++) {
      md = &optmds[i];
      if (md->sname == '\0') {
         continue;
      }
      if (md->sname == flag) {
         return md;
      }
   }

   return NULL;
}

static optmd_t *query_by_longname(const char *flag) {
   optmd_t *md;

   for (size_t i = 0; i < optmds_len; i++) {
      md = &optmds[i];
      if (md->lname == NULL) {
         continue;
      }
      if (!strcmp(md->lname, flag)) {
         return md;
      }
   }

   return NULL;
}

static argtok_t *gettok(argp_ctx_t *actx) {
   if (actx->idx == actx->toks_len) {
      issue_err(actx, "no command line argument left");
      exit(EXIT_FAILURE);
   }

   actx->tok = array_peek(actx->toks, actx->idx);
   actx->idx++;

   return actx->tok;
}

static void ungettok(argp_ctx_t *actx) {
   actx->idx--;
   actx->tok = array_peek(actx->toks, actx->idx);
}

static void preprocess_shortflag(argp_ctx_t *actx) {
   argtok_t *tok, *lookahead;

   tok = actx->tok;

   /* this option doesn't take a value */
   if (tok->md->reqv == false) {
      /* error path! */
      if (tok->value != NULL) {
         issue_err(actx,
            "-%s doesn't require a value but given one", tok->field);
      }
      return;
   }

   /* e.g. -j4 */
   if (tok->value != NULL) {
      return;
   }

   lookahead = gettok(actx);

   /* e.g. -j 4 */
   if (lookahead->kind == ArgtokPosArg) {
      /* tok->value is NULL, so okay to overwrite */
      tok->value = strdup(lookahead->field);
      return;
   }

   /* error path! */
   ungettok(actx);
   issue_err(actx, "no positional argument followed -%s", tok->field);
}

static void preprocess_longflag(argp_ctx_t *actx) {
   argtok_t *tok, *lookahead;

   tok = actx->tok;

   /* this option doesn't take a value */
   if (tok->md->reqv == false) {
      /* error path! */
      if (tok->value != NULL) {
         issue_err(actx,
            "--%s doesn't require a value but given one", tok->field);
      }
      return;
   }

   if (tok->has_eq) {
      /* e.g. --job=4 */
      if (tok->value != NULL) {
         return;
      }
      /* error path! */
      issue_err(actx, "value not given after --%s=", tok->field);
   }

   lookahead = gettok(actx);

   /* e.g. --job 4 */
   if (lookahead->kind == ArgtokPosArg) {
      /* tok->value is NULL, thus okay to overwrite */
      tok->value = strdup(lookahead->field);
      return;
   }

   /* error path! */
   ungettok(actx);
   issue_err(actx, "no positional argument followed --%s", tok->field);
}

static void handle_posarg(argp_ctx_t *actx) {
   array_append(cfg.source_files, actx->tok->field, strlen(actx->tok->field) + 1);
}

static void handle_help(argp_ctx_t *actx) {
   (void) actx;
   cfg.hlp = true;
}

static void handle_debug(argp_ctx_t *actx) {
   (void) actx;
   cfg.dbg = true;
}

static void handle_kawaii(argp_ctx_t *actx) {
   (void) actx;
   cfg.kwi = true;
}

static void handle_version(argp_ctx_t *actx) {
   (void) actx;
   cfg.vsn = true;
}

static void handle_dump(argp_ctx_t *actx) {
   (void) actx;
   cfg.dmp = true;
}

static void handle_optimize(argp_ctx_t *actx) {
   (void) actx;
   cfg.opt = true;
}

static void handle_direct(argp_ctx_t *actx) {
   (void) actx;
   cfg.drt = true;
}

static void handle_verbose(argp_ctx_t *actx) {
   (void) actx;
   cfg.vbs = true;
}

static void handle_dryrun(argp_ctx_t *actx) {
   (void) actx;
   cfg.drn = true;
}

static void handle_job(argp_ctx_t *actx) {
   char *end;
   unsigned long ret;

   errno = 0;
   ret = strtoul(actx->tok->value, &end, 10);

   if (errno == ERANGE || ret > ULONG_MAX) {
      issue_err(actx, "--job: a value out of range");
   }
   else
   if (end == actx->tok->value) {
      issue_err(actx, "--job: conversion not performed");
   }
   else
   if (end[0] != '\0') {
      issue_err(actx, "--job: value not fully consumed");
   }

   cfg.job = ret;
}

static void handle_output(argp_ctx_t *actx) {
   cfg.out = strdup(actx->tok->value);
}

static void handle_standard(argp_ctx_t *actx) {
   typedef struct {
      const char *spec;
      stdopt_t enum_val;
   } mapper_t;

   static const mapper_t accept_set[] = {
      { "spl01", StdoptSpl01 },
      { "cor27", StdoptCor27 }
   };
   static const size_t accept_set_len = ARRLEN(accept_set);

   const mapper_t *map;

   for (size_t i = 0; i < accept_set_len; i++) {
      map = &accept_set[i];
      if (!strcmp(actx->tok->value, map->spec)) {
         cfg.std = map->enum_val;
         return;
      }
   }

   issue_err(actx, "--std: unrecognized value '%s'", actx->tok->value);
}

static void handle_language(argp_ctx_t *actx) {
   typedef struct {
      const char *lang;
      lngopt_t enum_val;
   } mapper_t;

   static const mapper_t accept_set[] = {
      { "en", LngoptEn },
      { "ko", LngoptKo }
   };
   static const size_t accept_set_len = ARRLEN(accept_set);

   const mapper_t *map;

   for (size_t i = 0; i < accept_set_len; i++) {
      map = &accept_set[i];
      if (!strcmp(actx->tok->value, map->lang)) {
         cfg.lng = map->enum_val;
         return;
      }
   }

   issue_err(actx, "--lang: unrecognized value '%s'", actx->tok->value);
}

static void handle_target(argp_ctx_t *actx) {
   typedef struct {
      const char *target;
      tgtopt_t enum_val;
   } mapper_t;

   static const mapper_t accept_set[] = {
      { "c", TgtoptC },
   };
   static const size_t accept_set_len = ARRLEN(accept_set);

   const mapper_t *map;

   /* Note: since strcasecmp() is used, all the following
      would be treated as the same:
         haskell Haskell HASKELL etc... */

   for (size_t i = 0; i < accept_set_len; i++) {
      map = &accept_set[i];
      if (!strcasecmp(actx->tok->value, map->target)) {
         cfg.tgt = map->enum_val;
         return;
      }
   }

   issue_err(actx, "--target: unrecognized value '%s'", actx->tok->value);
}

static void handle_asmbly(argp_ctx_t *actx) {
   (void) actx;
   cfg.asmbly = true;
}

static void handle_disasm(argp_ctx_t *actx) {
   (void) actx;
   cfg.disasm = true;
}

static void handle_pedantic(argp_ctx_t *actx) {
   (void) actx;
   cfg.pdt = true;
}

static void handle_warning(argp_ctx_t *actx) {
   typedef struct {
      const char *request;
      bool *addr;
   } mapper_t;

   static const mapper_t accept_set[] = {
      { "keyword-case" , &cfg.w_kc },
      { "be-predicate" , &cfg.w_bp }
   };
   static const size_t accept_set_len = ARRLEN(accept_set);

   const mapper_t *map;

   for (size_t i = 0; i < accept_set_len; i++) {
      map = &accept_set[i];
      if (!strcmp(actx->tok->value, map->request)) {
         *map->addr = true;
         return;
      }
   }

   issue_err(actx, "-W: unrecognized value '%s'", actx->tok->value);
}

static void destruct_argtok(void *item, size_t idx) {
   argtok_t *tok;

   (void) idx;

   tok = item;
   free(tok->field);
   free(tok->value);
}

static void issue_err(argp_ctx_t *actx, const char *format, ...) {
   va_list ap;

   va_start(ap, format);
   safe_vprintf2(format, ap);
   safe_fputc(stdout, '\n');
   va_end(ap);

   actx->errcnt++;
}

/*** DEBUG ***/

static void showtoks(argp_ctx_t *actx, bool terse) {
   if (terse) {
      printf("\033[36;4midx\tkind\thas_eq\tseen\tfield\t\tvalue\033[0m\n");
      array_foreach(actx->toks, showtok_terse, NULL);
   }
   else
      array_foreach(actx->toks, showtok, NULL);
}

static void showtok(void *dat, size_t idx, void *ctx) {
   argtok_t *tok;

   (void) ctx;
   tok = dat;

   puts("==============================");
   printf("idx = %zu\n", idx);
   printf("kind = %s\n", resolve_argtok_kind(tok));
   printf("field = [%s]\n", tok->field ? tok->field : Cbblack "(empty)" Creset);
   printf("value = [%s]\n", tok->value ? tok->value : Cbblack "(empty)" Creset);
   printf("has_eq = %s\n", tok->has_eq ? "true" : "false");
   if (tok->md) {
      printf("\ncalled [-%c] or [--%s]\n", tok->md->sname, tok->md->lname);
      printf("seen count = %zu\n", tok->md->seen);
      printf("value-taking = %s\n", tok->md->reqv ? "true" : "false");
      printf("unique = %s\n", tok->md->unique ? "true" : "false");
   }
   putchar('\n');
}

static void showtok_terse(void *dat, size_t idx, void *ctx) {
   argtok_t *tok;

   (void) ctx;
   tok = dat;

   printf("%zu\t%s\t%s\t",
      idx,
      resolve_argtok_kind(tok),
      tok->has_eq ? "true" : "false"
   );

   if (tok->md)
      printf("%zu", tok->md->seen);
   else
      printf(Cbblack "(n/a)" Creset);

   printf("\t%s\t\t", tok->field ? tok->field : Cbblack "(n/a)" Creset);
   printf("%s\n", tok->value ? tok->value : Cbblack "(empty)" Creset);
}

static const char *resolve_argtok_kind(argtok_t *tok) {
   switch (tok->kind) {
      case ArgtokShortFlag : return "short";
      case ArgtokLongFlag  : return "long";
      case ArgtokPosArg    : return "posarg";
      case ArgtokOptionEnd : return "--";
      default: ;  /* control never reaches here */
   }
   return NULL;
}

static void list_options(void) {
   size_t i;

   puts("# SHORT FLAGS");
   for (i = 0; i < optmds_len; i++) {
      if (optmds[i].sname == '\0')
         continue;
      printf("   %c (%s)\n", optmds[i].sname, optmds[i].lname);
   }

   puts("\n# LONG FLAGS");
   for (i = 0; i <optmds_len; i++) {
      if (optmds[i].lname == NULL)
         continue;
      printf("   %s\n", optmds[i].lname);
   }
}
