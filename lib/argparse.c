#include "argparse.h"
#include "argparse.internals.h"

/*
Improvement Ideas
   - argument taking optional value
      -O => uses its default value
      -O3 => uses 3 as its value
*/

extern bool parse_argv(
   int argc,
   const char *argv[],
   optmd_t table[],
   size_t table_len,
   array_t *sourcefiles
) {
   bool parse_ret = false;
   arena_t *arena = NULL;
   array_t *toks = NULL;
   array_t *err_infos = NULL;
   argp_ctx_t *actx;

   /* Inits actx */
   if (!(arena = arena_create()))
      goto cleanup;
   if (!(actx = arena_malloc(arena, sizeof *actx)))
      goto cleanup;
   if (!(toks = array_create(destruct_argtok, arena)))
      goto cleanup;
   if (!(err_infos = array_create(destruct_errinfo, arena)))
      goto cleanup;

   actx->arena = arena;
   actx->argc = argc;
   actx->argv = argv;
   actx->arg = NULL;
   actx->toks = toks;
   actx->tok = NULL;
   actx->toks_len = 0;
   actx->idx = 0;
   actx->eoe = false;
   actx->table = table;
   actx->table_len = table_len;
   actx->sourcefiles = sourcefiles;
   actx->errcnt = 0;
   actx->errcnt_max = 20;
   actx->err = (argerr_t) {
      .errhandle = argp_errhandle_default,
      .reason = "none",
      .ctx = NULL,
      .ctx_siz = 0
   };
   actx->err_infos = err_infos;
   actx->lookahead_ret = false;
   actx->preprocess_ret = false;

   /* Main logic */
   if (validate_schemas(actx) != ArgpStatOk)
      goto cleanup;
   if (tokenize_argv(actx) != ArgpStatOk)
      goto cleanup;
   if (parse_argtoks(actx) != ArgpStatOk)
      goto cleanup;

   if (actx->errcnt > 0) {
      if (!report_errors(actx)) {
         goto cleanup;
      }
   }
   else parse_ret = true;

cleanup:
   array_destroy(err_infos);
   array_destroy(toks);
   arena_destroy(arena);
   return parse_ret;
}

extern bool usage(
   optmd_t table[],
   size_t table_len,
   const char *intro
) {
   int nchar;

   nchar = printf("%s\n\n", intro);
   if (nchar < 0) return false;

   nchar = printf("OPTIONS\n");
   if (nchar < 0) return false;

   for (size_t i = 0; i < table_len; i++) {
      nchar = printf("\t" Cyellow);
      if (nchar < 0) return false;

      if (table[i].sname != '\0' && table[i].lname == NULL) {
         nchar = printf("-%c" Creset, table[i].sname);
         if (nchar < 0) return false;
      }
      else
      if (table[i].sname == '\0' && table[i].lname != NULL) {
         nchar = printf("--%s" Creset, table[i].lname);
         if (nchar < 0) return false;
      }
      else {
         nchar = printf(
            "-%c" Creset ", " Cyellow "--%s" Creset,
            table[i].sname, table[i].lname);
         if (nchar < 0) return false;
      }

      nchar = printf("\n\t\t%s\n", table[i].help);
      if (nchar < 0) return false;
   }

   nchar = printf(
      "\n"
      "tip: short flags can be specified in a"
         " row like " Ccyan "-xvf" Creset "\n"
      "tip: " Ccyan "--flag value" Creset
         " is the same as " Ccyan "--flag=value" Creset "\n"
      "tip: " Ccyan "-f value" Creset " is"
         " the same as " Ccyan "-fvalue" Creset "\n"
      "tip: use " Ccyan "--" Creset " to specify the end of option\n"
   );
   if (nchar < 0) return false;
}

static argp_stat_t validate_schemas(argp_ctx_t *actx) {
   size_t errcnt = 0;
   optmd_t *subject, *against;
   size_t i, j;
   int nchar;

   /* option table is empty? */
   if (actx->table_len == 0) {
      (void) printf("the option table is empty.\n");
      /* whether this printf is successful or not,
         this path returns Err always */
      return ArgpStatErr;
   }

   /* duplication check for short flags */
   for (i = 0; i < actx->table_len; i++) {
      subject = &actx->table[i];
      if (subject->sname == '\0')
         continue;
      for (j = i + 1; j < actx->table_len; j++) {
         against = &actx->table[j];
         if (against->sname == '\0')
            continue;
         if (subject->sname == against->sname) {
            nchar = printf(
               "md[%zu] has the same short name with md[%zu].\n", i, j);
            if (nchar < 0) return ArgpStatErr;
            errcnt++;
         }
      }
   }

   /* checks if long flag is NULL */
   for (i = 0; i < actx->table_len; i++) {
      subject = &actx->table[i];
      if (subject->lname == NULL) {
         nchar = printf("no long name is given for md[%zu].\n", i);
         if (nchar < 0) return ArgpStatErr;
         errcnt++;
      }
   }

   /* duplication check for long flags */
   for (i = 0; i < actx->table_len; i++) {
      subject = &actx->table[i];
      if (subject->lname == NULL) {
         /* the preceding validation doesn't mean that
            the flags do have their long names -- so
            this check is necessary */
         continue;
      }
      for (j = i + 1; j < actx->table_len; j++) {
         against = &actx->table[j];
         if (against->lname == NULL)
            continue;
         if (!strcmp(subject->lname, against->lname)) {
            nchar = printf(
               "md[%zu] has the same long name with md[%zu].\n", i, j);
            if (nchar < 0) return ArgpStatErr;
            errcnt++;
         }
      }
   }

   if (errcnt > 0) {
      /* No choice but to kill it! */
      (void) printf(
         "Currently the program has a problematic option table. "
         "Terminating.\n"
      );
      return ArgpStatErr;
   }
   else {
      return ArgpStatOk;
   }
}

static argp_stat_t tokenize_argv(argp_ctx_t *actx) {
   /* Note: begins from 1 to skip the program name */
   for (int i = 1; i < actx->argc; i++) {
      actx->arg = actx->argv[i];

      if (actx->eoe) {
         if (!populate_as_posarg(actx)) {
            return ArgpStatErr;
         }
         continue;
      }

      if (!strcmp(actx->arg, "--")) {
         actx->eoe = true;
         if (!populate_as_dashdash(actx)) {
            return ArgpStatErr;
         }
      }
      else
      if (!strncmp(actx->arg, "--", 2)) {
         if (!populate_as_longflag(actx)) {
            return ArgpStatErr;
         }
      }
      else
      if (actx->arg[0] == '-' && actx->arg[1] != '\0') {
         if (!populate_as_shortflag(actx)) {
            return ArgpStatErr;
         }
      }
      else {
         if (!populate_as_posarg(actx)) {
            return ArgpStatErr;
         }
      }
   }
   actx->toks_len = array_size(actx->toks);

   return ArgpStatOk;
}

static argp_stat_t parse_argtoks(argp_ctx_t *actx) {
   argp_stat_t stat;
   argtok_t *tok;
   bool parse_result;


   /* Note: toks has at least one element since argc > 1 supposed */
   /* Did not use gettok() on purpose; since it advances actx->idx,
      causing an unmatch between current token and its index */
   tok = actx->tok = array_peek(actx->toks, 0);

   while (actx->idx < actx->toks_len) {
      /* end of option? */
      if (tok->kind == ArgtokOptionEnd) {
         break;
      }

      /* positional argument? */
      if (tok->kind == ArgtokPosArg) {
         if (!handle_posarg(actx)) {
            return ArgpStatErr;
         }
         goto next;
      }

      /* unrecognized option? */
      if (tok->md == NULL) {
         actx->err.errhandle = errhandle_unrecognized;
         actx->err.ctx = & (errhandle_unrecognized_ctx_t) {
            .table = actx->table,
            .table_len = actx->table_len
         };
         actx->err.ctx_siz = sizeof (errhandle_unrecognized_ctx_t);
         if ((stat = regerr(actx, tok)) != ArgpStatOk) {
            return stat;  /* Err or ErrcntFull */
         }
         goto next;
      }

      /* Since it's neither -- nor posarg, it's either a
         valid short or long flag, meaning tok->md must exist */

      /* specified multiple times? */
      if (tok->md->seen++ && tok->md->unique == true) {
         actx->err.errhandle = errhandle_seen_multiple;
         actx->err.ctx = tok->md;
         actx->err.ctx_siz = sizeof *tok->md;
         if ((stat = regerr(actx, tok)) != ArgpStatOk) {
            return stat;
         }
         goto next;
      }

      switch (tok->kind) {
         /* to be accepted: "-j" | "-j4" | "-j 4" */
         case ArgtokShortFlag: {
            if ((stat = preprocess_shortflag(actx)) != ArgpStatOk) {
               return stat;
            }
         }

         /* to be accepted: "--job" | "--job=4" | "--job 4" */
         case ArgtokLongFlag: {
            if ((stat = preprocess_longflag (actx)) != ArgpStatOk) {
               return stat;
            }
         }

         /* control never reaches default */
         default: ;
      }
      if (actx->preprocess_ret == false) {
         goto next;
      }

      /* Even if gettok() called once again in preprocess(),
         the value of this 'tok' would not be changed since
         it would have been assigned at the head of 'while' */
      parse_result = (*tok->md->drive)(tok, &actx->err);
      if (parse_result == false) {
         /* err has been filled by drive() */
         if ((stat = regerr(actx, tok)) != ArgpStatOk) {
            return stat;
         }
      }

   next:
      if (actx->idx == actx->toks_len - 1) {
         break;  /* end of token */
      }
      if ((stat = gettok(actx)) != ArgpStatOk) {
         return stat;
      }
   }

   /* Collects the remanining positional arguments after -- as filenames */
   while (actx->idx < actx->toks_len - 1) {
      if ((stat = gettok(actx)) != ArgpStatOk) {
         return stat;
      }
      if (!handle_posarg(actx)) {
         return ArgpStatErr;
      }
   }

   return ArgpStatOk;
}

static bool handle_posarg(argp_ctx_t *actx) {
   return array_append(
      actx->sourcefiles,
      actx->tok->field,
      strlen(actx->tok->field) + 1
   );
}

static bool populate_as_dashdash(argp_ctx_t *actx) {
   argtok_t tok = {
      .kind = ArgtokOptionEnd,
      .field = NULL,
      .value = NULL,
      .md = NULL,
      .has_eq = false
   };

   return array_append(actx->toks, &tok, sizeof tok);
}

static bool populate_as_longflag(argp_ctx_t *actx) {
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
   if (!tok.field) {
      goto err1;
   }
   tok.md = query_by_longname(actx, tok.field);

   if (eq_pos < arg_len) {
      tok.has_eq = true;
      /* arg_len - eq_pos == 1 happens when no value given after = */
      if (arg_len - eq_pos > 1) {
         tok.value = strdup(&actx->arg[eq_pos + 1]);
         if (!tok.value) {
            goto err2;
         }
      }
   }

   /* normal path */
   return array_append(actx->toks, &tok, sizeof tok);

err2:
   free(tok.field);
err1:
   return false;
}

static bool populate_as_shortflag(argp_ctx_t *actx) {
   argtok_t tok = {
      .kind = ArgtokShortFlag,
      .field = NULL,
      .value = NULL,
      .md = NULL,
      .has_eq = false
   };
   size_t arg_len;

   actx->arg++;  /* skips - */
   arg_len = strlen(actx->arg);

   /* Advances actx->arg one letter at a time
         -abcoa.out
          0123^~~~~ */
   for (char flag; (flag = actx->arg[0]) != '\0'; actx->arg++, arg_len--) {
      /* Copies one letter as a string */
      tok.field = strndup(actx->arg, 1);
      if (!tok.field) {
         goto err1;
      }

      /* Looks up the option table to know whether this is
         a value-taking option */
      tok.md = query_by_shortname(actx, flag);

      if (tok.md != NULL) {
         if (tok.md->reqv && arg_len > 1) {
            /* Treats the rest of 'arg' as this short flag's value.
               For example:
                  -abcoa.out
                      ^^~~~~
               since -o is a value-taking option */
            tok.value = strdup(&actx->arg[1]);
            if (!tok.value) {
               goto err2;
            }
         }
      }

      if (!array_append(actx->toks, &tok, sizeof tok)) {
         goto err3;
      }

      /* if this flag were a value-taking option, the rest of 'arg'
         had been consumed as its value */
      if (tok.md != NULL && tok.md->reqv) {
         return true;  /* happy path: -abcoa.out */
      }
   }

   return true;  /* happy path: -abc */

err3:
   free(tok.value);  /* safe, since it was NULL-init'ed */
err2:
   free(tok.field);  /* safe, also. */
err1:
   return false;
}

static bool populate_as_posarg(argp_ctx_t *actx) {
   argtok_t tok = {
      .kind = ArgtokPosArg,
      .field = strdup(actx->arg),
      .value = NULL,
      .md = NULL,
      .has_eq = false
   };

   return array_append(actx->toks, &tok, sizeof tok);
}

static optmd_t *query_by_shortname(argp_ctx_t *actx, char flag) {
   optmd_t *md;

   for (size_t i = 0; i < actx->table_len; i++) {
      md = &actx->table[i];
      if (md->sname == '\0') {
         continue;
      }
      if (md->sname == flag) {
         return md;
      }
   }

   return NULL;
}

static optmd_t *query_by_longname(argp_ctx_t *actx, const char *flag) {
   optmd_t *md;

   for (size_t i = 0; i < actx->table_len; i++) {
      md = &actx->table[i];
      if (md->lname == NULL) {
         continue;
      }
      if (!strcmp(md->lname, flag)) {
         return md;
      }
   }

   return NULL;
}

[[nodiscard]]
static argp_stat_t gettok(argp_ctx_t *actx) {
   argp_stat_t stat;

   /* reached end of token? */
   if (actx->idx == actx->toks_len - 1) {
      actx->err.errhandle = errhandle_out_of_tok;
      /* Note: actx->tok is meaningless -- just placeholder */
      if ((stat = regerr(actx, actx->tok)) != ArgpStatOk) {
         return stat;
      }
      return ArgpStatErr;  /* end of token */
   }

   actx->idx++;
   actx->tok = array_peek(actx->toks, actx->idx);

   return ArgpStatOk;
}

static void ungettok(argp_ctx_t *actx) {
   actx->idx--;
   actx->tok = array_peek(actx->toks, actx->idx);
}

static argp_stat_t preprocess_shortflag(argp_ctx_t *actx) {
   argp_stat_t stat;
   argtok_t *tok = actx->tok;

   /* This option doesn't take a value */
   if (tok->md->reqv == false) {
      /* Note: tok->value must be NULL in this case
         due to the logic of populate_as_shortflag() */
      actx->preprocess_ret = true;
      return ArgpStatOk;
   }

   /* e.g. -j4 */
   if (tok->value != NULL) {
      actx->preprocess_ret = true;
      return ArgpStatOk;
   }

   /* e.g. -j 4 */
   if ((stat = lookahead(actx)) != ArgpStatOk) {
      return stat;
   }
   if (actx->lookahead_ret) {
      actx->preprocess_ret = true;
      return ArgpStatOk;
   }

   /* Expected one, but no positional argument existed */
   actx->preprocess_ret = false;
   ungettok(actx);

   actx->err.errhandle = argp_errhandle_default;
   actx->err.reason = "no positional argument follows";

   return regerr(actx, actx->tok);
}

static argp_stat_t preprocess_longflag(argp_ctx_t *actx) {
   argp_stat_t stat;
   argtok_t *tok = actx->tok;

   /* This option doesn't take a value */
   if (tok->md->reqv == false) {
      if (tok->value != NULL) {
         actx->preprocess_ret = false;
         actx->err.errhandle = argp_errhandle_default;
         actx->err.reason = "doesn't require a value but given one";
         if ((stat = regerr(actx, tok)) != ArgpStatOk) {
            return stat;
         }
         return ArgpStatOk;
      }
      if (tok->has_eq) {
         actx->preprocess_ret = false;
         actx->err.errhandle = argp_errhandle_default;
         actx->err.reason = "'=' is used with a non-value-taking option";
         if ((stat = regerr(actx, tok)) != ArgpStatOk) {
            return stat;
         }
         return ArgpStatOk;
      }
      /* A valid non-value-taking option */
      actx->preprocess_ret = true;
      return ArgpStatOk;
   }

   if (tok->has_eq) {
      /* e.g. --job= */
      if (tok->value == NULL) {
         actx->preprocess_ret = false;
         actx->err.errhandle = argp_errhandle_default;
         actx->err.reason = "value not given after '='";
         if ((stat = regerr(actx, tok)) != ArgpStatOk) {
            return stat;
         }
         return ArgpStatOk;
      }
      /* e.g. --job=4 */
      else {
         actx->preprocess_ret = true;
         return ArgpStatOk;
      }
   }

   /* e.g. --job 4 */
   if ((stat = lookahead(actx)) != ArgpStatOk) {
      return stat;
   }
   if (actx->lookahead_ret) {
      actx->preprocess_ret = true;
      return ArgpStatOk;
   }

   /* Expected one, but no positional argument existed */
   actx->preprocess_ret = false;
   ungettok(actx);

   actx->err.errhandle = argp_errhandle_default;
   actx->err.reason = "no positional argument follows";

   return regerr(actx, actx->tok);
}

static argp_stat_t lookahead(argp_ctx_t *actx) {
   argp_stat_t stat;
   argtok_t *curr, *lookahead;

   curr = actx->tok;
   if ((stat = gettok(actx)) != ArgpStatOk) {
      return stat;
   }
   lookahead = actx->tok;

   if (lookahead->kind == ArgtokPosArg) {
      /* tok->value is NULL, thus okay to overwrite */
      if (!(curr->value = strdup(lookahead->field))) {
         return ArgpStatErr;
      }
      actx->lookahead_ret = true;
   }
   else {
      actx->lookahead_ret = false;
   }

   return ArgpStatOk;
}

static void destruct_argtok(void *item, size_t idx) {
   argtok_t *tok;

   (void) idx;
   tok = item;
   free(tok->value);
   free(tok->field);
}

static void destruct_errinfo(void *item, size_t idx) {
   errinfo_t *ei;

   (void) idx;
   ei = item;
   free(ei->tok.field);
   free(ei->tok.value);
}

extern bool argp_errhandle_default(argtok_t *tok, const char *reason, void *ctx) {
   /* Note: for short flag and long flag, whose tok is fully filled */
   int nchar;

   (void) ctx;
   nchar = printf("%s: %s\n", tok->field, reason);

   return (0 <= nchar) ? true : false;
}

static bool errhandle_seen_multiple(argtok_t *tok, const char *reason, void *ctx) {
   /* Note: for short flag and long flag, whose tok is fully filled */
   int nchar;
   optmd_t *md;

   (void) reason;
   md = ctx;

   nchar = printf(
      "%s: already specified %zu times\n",
      tok->field,
      md->seen
   );

   return (0 <= nchar) ? true : false;
}

static bool errhandle_unrecognized(argtok_t *tok, const char *reason, void *ctx) {
   /* Note: only tok.field is available since this is not a valid option */
   static bool once = false;
   static const char *err_short = "unrecognized short flag";
   static const char *err_long  = "unrecognized long flag";

   errhandle_unrecognized_ctx_t *ectx = ctx;
   find_alikes_ret_t ret = { .alikes = NULL };
   bool handle_ret = false;
   int nchar;

   (void) reason;
   ret = find_alikes(
      tok->field,
      ectx->table,
      ectx->table_len,
      sizeof ESIZ(ectx->table)
   );
   if (!ret.alikes) {
      return false;
   }

   if (tok->kind == ArgtokShortFlag) {
      nchar = printf("%s: %s\n", tok->field, err_short);
      if (nchar < 0) goto cleanup;
   }
   else /* long flag */ {
      nchar = printf("%s: %s. Do you mean ", tok->field, err_long);
      if (nchar < 0) goto cleanup;

      /* at least one exists */
      if (ret.nmemb == 1) {
         nchar = printf(Cbgreen "--%s" Creset "?\n", ret.alikes[0]);
         if (nchar < 0) goto cleanup;
      }
      else {
         nchar = printf("one of the following?\n\t" Cbgreen);
         if (nchar < 0) goto cleanup;

         for (size_t i = 0; i < ret.nmemb; i++) {
            nchar = printf("%s  ", ret.alikes[i]);
            if (nchar < 0) goto cleanup;
         }
         nchar = printf(Creset "\n");
         if (nchar < 0) goto cleanup;
      }
   }

   if (!once) {
      nchar = printf(
         "* Note: should this be a filename, specify '"
         Cbwhite "--" Creset "' first\n"
         "* Note: type "
         Cbmagenta "-h" Creset " or "
         Cbmagenta "--help" Creset " to see the manual page\n"
      );
      if (nchar < 0) goto cleanup;
      once = true;
   }

   handle_ret = true;

cleanup:
   free(ret.alikes);

   return handle_ret;
}

static bool errhandle_out_of_tok(argtok_t *tok, const char *reason, void *ctx) {
   int nchar;

   /* Note: can't use tok in this context */
   (void) tok, (void) reason, (void) ctx;

   nchar = printf(
      "argparse: ran out of command line arguments, "
      "having expected one\n"
   );

   return (0 <= nchar) ? true : false;
}

static argp_stat_t regerr(argp_ctx_t *actx, argtok_t *tok) {
   errinfo_t ei = {
      .tok = *tok,
      .err = actx->err,  /* structs are copied */
      .ctx = NULL
   };

   /* Inits ei.tok */
   if (tok->field != NULL) {
      if (!(ei.tok.field = strdup(tok->field))) {
         goto err1;
      }
   }
   if (tok->value != NULL) {
      if (!(ei.tok.value = strdup(tok->value))) {
         goto err2;
      }
   }

   /* Inits ei.ctx */
   if (actx->err.ctx != NULL) {
      if (!(ei.ctx = arena_malloc(actx->arena, actx->err.ctx_siz))) {
         goto err3;
      }
      memcpy(ei.ctx, actx->err.ctx, actx->err.ctx_siz);
   }

   if (!array_append(actx->err_infos, &ei, sizeof ei)) {
      goto err4;
   }
   actx->errcnt++;

   /* Resets .ctx */
   actx->err.ctx = NULL;

   if (actx->errcnt == actx->errcnt_max) {
      return ArgpStatErrcntFull;
   }
   else {
      return ArgpStatOk;
   }

err4:
   ;  /* ei.ctx used arena */
err3:
   free(ei.tok.value);
err2:
   free(ei.tok.field);
err1:
   return ArgpStatErr;
}

static bool report_errors(argp_ctx_t *actx) {
   size_t len;
   errinfo_t *ei;
   int nchar;

   len = array_size(actx->err_infos);

   for (size_t i = 0; i < len; i++) {
      ei = array_peek(actx->err_infos, i);
      if (!ei->err.errhandle(&ei->tok, ei->err.reason, ei->ctx)) {
         return false;
      }
   }

   if (actx->errcnt < actx->errcnt_max) {
      return true;
   }

   nchar = printf("error count reached max."
      " Aborting argv parsing at idx = %zu.\n",
      actx->idx
   );

   return (0 <= nchar) ? true : false;
}

/*** DEBUG ***/

[[maybe_unused]]
static void showargtoks(argp_ctx_t *actx, bool terse) {
   if (terse) {
      printf(Ccyan Cul "idx\tkind\thas_eq\tseen\tfield\t\tvalue\n" Creset);
      array_foreach(actx->toks, showargtok_terse, NULL);
   }
   else
      array_foreach(actx->toks, showargtok, NULL);
}

static void showargtok(void *dat, size_t idx, void *ctx) {
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

static void showargtok_terse(void *dat, size_t idx, void *ctx) {
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

[[maybe_unused]]
static void list_options(argp_ctx_t *actx) {
   size_t i;

   puts("## SHORT FLAGS ##");
   for (i = 0; i < actx->table_len; i++) {
      if (actx->table[i].sname == '\0')
         continue;
      printf("\t%c\t(%s)\n", actx->table[i].sname, actx->table[i].lname);
   }

   puts("\n## LONG FLAGS ##");
   for (i = 0; i < actx->table_len; i++) {
      if (actx->table[i].lname == NULL)
         continue;
      printf("\t%s\n", actx->table[i].lname);
   }
}
