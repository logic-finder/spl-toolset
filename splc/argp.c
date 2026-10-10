#include <stdbool.h>
#include <string.h>

#include "splc.h"
#include "wrappers.h"
#include "lib/strutils.h"
#include "array.adt.h"

typedef void (*opt_parser_t)(void);

typedef struct {
   char sname;  /* short name */
   const char *lname;  /* long name */
   opt_parser_t parse;
   void *value;
   const char *help;
   bool seen;
} optstat_t;

typedef struct {
   bool help;
   array_t *source_files;
} config_t;

typedef enum {
   ArgtokShortFlag,
   ArgtokLongFlag,
   ArgtokOptionEnd,
   ArgtokPosArg
} argtok_kind_t;

typedef struct {
   argtok_kind_t kind;
   char *field;
   char *value;
} argtok_t;

typedef struct {
   int argc;
   const char **argv;
   array_t *toks;
   size_t toks_len;
   bool eoe;
} argp_ctx_t;

config_t cfg;

optstat_t optstats[] = {
   {
      .sname = 'h',
      .lname = "help",
      .parse = parse_help,
      .value = &cfg.help,
      .help  = "show the list of cli options"
   }
};

extern void parse_args(compile_ctx_t *cctx) {
   argp_ctx_t *actx;
   argtok_t *tok;

   actx = safe_malloc(sizeof *actx);
   actx->argc = cctx->argc;
   actx->argv = cctx->argv;
   actx->toks = array_create(destruct_argtok);
   actx->eoe  = false;

   tokenize_argv(actx);

   for (size_t i = 0; actx->toks_len; i++) {
      tok = array_peek(actx->toks, i);

      if (actx->eoe) {
         parse_posarg();
         continue;
      }

      switch (tok->kind) {
         case ArgtokShortFlag : parse_shortflag(); break;
         case ArgtokLongFlag  : parse_longflag (); break;
         case ArgtokOptionEnd : parse_optionend(); break;
         case ArgtokPosArg    : parse_posarg   (); break;
         default: ;  /* control never reaches default */
      }
   }
}

static void destruct_argtok(void *item, size_t idx) {

}

static void tokenize_argv(argp_ctx_t *actx) {
   const char *arg;
   argtok_t tok;

   /* Note: begins from 1 to skip the program name */
   for (int i = 1; i < actx->argc; i++) {
      arg = actx->argv[i];
      if (arg[0] == '-' && arg[1] == '-' && arg[2] == '\0') {
         populate_as_dashdash(&tok, arg);
      }
      else
      if (arg[0] == '-' && arg[1] == '-') {
         populate_as_longflag(&tok, arg);
      }
      else
      if (arg[0] == '-') {
         populate_as_shortflag(&tok, arg);
      }
      else {
         populate_as_posarg(&tok, arg);
      }
      array_append(actx->toks, &tok, sizeof tok);
   }
   actx->toks_len = array_size(actx->toks);
}

static void populate_as_dashdash(argtok_t *dest, const char *arg) {
   (void) arg;
   dest->kind = ArgtokOptionEnd;
   dest->field = NULL;
   dest->value = NULL;
}

static void populate_as_longflag(argtok_t *dest, const char *arg) {
   size_t eq_pos, field_len, value_len;

   dest->kind = ArgtokLongFlag;

   /*   01234567890123
      --target=haskell
        ^      ^
        field  value */

   arg += 2;  /* skips -- */
   eq_pos = strcspn(arg, "=");

   dest->field = strndup(arg, eq_pos);

   /* does not have '=' ? */
   if (eq_pos == strlen(arg)) {
      dest->value = NULL;
      return;
   }

   dest->value = strdup(&arg[eq_pos + 1]);
}

static void populate_as_shortflag(argtok_t *dest, const char *arg) {
   arg++;  /* skips - */
   dest->kind = ArgtokShortFlag;
   dest->field = strdup(arg);
   dest->value = NULL;
}

static void populate_as_posarg(argtok_t *dest, const char *arg) {
   dest->kind = ArgtokPosArg;
   dest->field = strdup(arg);
   dest->value = NULL;
}
