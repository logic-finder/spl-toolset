#include "argp.h"
#include "argp.internals.h"

extern void parse_args(compile_ctx_t *cctx) {
   volatile argp_ctx_t actx = {0};
   int ret;

   actx.argc = cctx->argc;
   actx.argv = cctx->argv;
   actx.toks = array_create(destruct_argtok);
   actx.idx = 0;
   actx.eoe  = false;
   actx.errors = array_create(destruct_error);
   actx.reason = "none";

   ret = setjmp(actx.env);
   if (ret != 0) {
      goto report_error;
   }

   init_config();

   // TODO: validate_optmd_schemas()
   tokenize_argv(&actx);
   parse_argtoks(&actx);
   // TODO: validate_argtoks()

report_error: ;
   // TODO: report_errors()

   // TODO: cctx->cfg = &cfg;
}

static void init_config(void) {
   cfg.source_files = array_create(destruct_sourcefile);
   cfg.job = 1;
   cfg.out = "a.out";
   cfg.std = StdoptSpl01;
   cfg.lng = LngoptEn;
}

static void parse_argtoks(argp_ctx_t *actx) {
   argtok_t *tok;

   while (actx->idx < actx->toks_len) {
      tok = gettok(actx);

      if (tok->kind == ArgtokOptionEnd) {
         break;
      }
      if (tok->kind == ArgtokPosArg) {
         handle_posarg(actx);
      }

      if (tok->kind != ArgtokShortFlag || tok->kind != ArgtokLongFlag) {
         continue;
      }

      /* ? Note: tok->md must exist since it's a valid flag */

      /* specified multiple times? */
      if (tok->md->seen++) {
         actx->reason = "already specified %zu times";
         regerr(actx);
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

      (*tok->md->handle)(actx);
   }

   // 남은거 수집 (있으면)
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
      actx->reason = "unknown long flag";
      regerr(actx);
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

   while ((flag = actx->arg[0]) != '\0') {
      tok.md = query_by_shortname(flag);

      if (tok.md == NULL) {
         actx->reason = "unknown short flag";
         regerr(actx);
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

      actx->arg++;
      arg_len--;
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
}

static argtok_t *gettok(argp_ctx_t *actx) {
   if (actx->idx == actx->toks_len - 1) {
      actx->reason = "expected one, but no argument left";
      regerr(actx);
      longjmp(actx->env, 1);
   }

   actx->idx++;
   actx->tok = array_peek(actx->toks, actx->idx);

   return actx->tok;
}

static void ungettok(argp_ctx_t *actx) {
   actx->idx--;
   actx->tok = array_peek(actx->toks, actx->idx);
}

static void preprocess_shortflag(argp_ctx_t *actx) {
   argtok_t *current, *lookahead;

   /* this option doesn't take a value */
   if (actx->tok->md->reqv == false) {
      return;
   }

   /* e.g. -j4 */
   if (actx->tok->value != NULL) {
      return;
   }

   current = actx->tok;
   lookahead = gettok(actx);

   /* e.g. -j 4 */
   if (lookahead->kind == ArgtokPosArg) {
      /* current->value is NULL, so okay to overwrite */
      current->value = strdup(lookahead->field);
      return;
   }

   /* error path! */
   ungettok(actx);
   actx->reason = "no positional argument followed a short flag";
   regerr(actx);
}

static void preprocess_longflag(argp_ctx_t *actx) {
   argtok_t *current, *lookahead;

   /* this option doesn't take a value */
   if (actx->tok->md->reqv == false) {
      return;
   }

   if (actx->tok->has_eq) {
      /* e.g. --job=4 */
      if (actx->tok->value != NULL) {
         return;
      }
      /* error path! */
      actx->reason = "value not given after =";
      regerr(actx);
   }

   current = actx->tok;
   lookahead = gettok(actx);

   /* e.g. --job 4 */
   if (lookahead->kind == ArgtokPosArg) {
      /* current->value is NULL, so okay to overwrite */
      current->value = strdup(lookahead->field);
      return;
   }

   /* error path! */
   ungettok(actx);
   actx->reason = "no positional argument followed a long flag";
   regerr(actx);
}

static void handle_posarg(argp_ctx_t *actx) {
   array_append(cfg.source_files, actx->tok->field
}

static void handle_help(argp_ctx_t *actx) {
   cfg.hlp = true;
}

static void handle_version(argp_ctx_t *actx) {

}

static void destruct_sourcefile(void *item, size_t idx) {

}

static void destruct_argtok(void *item, size_t idx) {

}

static void destruct_error(void *item, size_t idx) {

}

static void regerr(argp_ctx_t *actx) {

}
