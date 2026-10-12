#ifndef ARGP_INTERNALS_H
#define ARGP_INTERNALS_H

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdarg.h>

#include "fatal.h"
#include "common.h"
#include "wrappers.h"
#include "strutils.h"
#include "array.adt.h"
#include "colorcodes.h"

typedef struct argtok argtok_t;

typedef struct {
   int argc;
   const char **argv;
   const char *arg;
   array_t *toks;
   argtok_t *tok;
   size_t toks_len;
   size_t idx;
   bool eoe;
   size_t errcnt;
} argp_ctx_t;

typedef void opt_handler_t(argp_ctx_t *actx);

typedef struct {
   char sname;  /* short name */
   const char *lname;  /* long name */
   opt_handler_t *handle;
   bool reqv;
   const char *help;
   size_t seen;  /* works as a counter */
   bool unique;
} optmd_t;

typedef enum {
   ArgtokShortFlag,
   ArgtokLongFlag,
   ArgtokOptionEnd,
   ArgtokPosArg
} argtok_kind_t;

struct argtok {
   argtok_kind_t kind;
   char *field;
   char *value;
   optmd_t *md;
   bool has_eq;  /* only meaningful in the long-flag context */
};

static opt_handler_t handle_posarg;
static opt_handler_t handle_help;
static opt_handler_t handle_debug;
static opt_handler_t handle_kawaii;
static opt_handler_t handle_version;
static opt_handler_t handle_dump;
static opt_handler_t handle_optimize;
static opt_handler_t handle_direct;
static opt_handler_t handle_verbose;
static opt_handler_t handle_dryrun;
static opt_handler_t handle_job;
static opt_handler_t handle_output;
static opt_handler_t handle_standard;
static opt_handler_t handle_language;
static opt_handler_t handle_target;
static opt_handler_t handle_asmbly;
static opt_handler_t handle_disasm;
static opt_handler_t handle_pedantic;
static opt_handler_t handle_warning;

static void init_config(void);
static void validate_schemas(void);
static void tokenize_argv(argp_ctx_t *actx);
static void parse_argtoks(argp_ctx_t *actx);

static void populate_as_dashdash(argp_ctx_t *actx);
static void populate_as_longflag(argp_ctx_t *actx);
static void populate_as_shortflag(argp_ctx_t *actx);
static void populate_as_posarg(argp_ctx_t *actx);

static optmd_t *query_by_shortname(char flag);
static optmd_t *query_by_longname(const char *flag);

static argtok_t *gettok(argp_ctx_t *actx);
static void ungettok(argp_ctx_t *actx);

static void preprocess_shortflag(argp_ctx_t *actx);
static void preprocess_longflag(argp_ctx_t *actx);

static void destruct_sourcefile(void *item, size_t idx);
static void destruct_argtok(void *item, size_t idx);

static void issue_err(argp_ctx_t *actx, const char *format, ...);

static void showtoks(argp_ctx_t *actx, bool terse);
static void showtok(void *dat, size_t idx, void *ctx);
static void showtok_terse(void *dat, size_t idx, void *ctx);
static const char *resolve_argtok_kind(argtok_t *tok);

static void list_options(void);

/*==================*
 | STATIC VARIABLES |
 *==================*/
static optmd_t optmds[] = {
   /* Note: .seen be initialized with 0 */
   {
      .sname = 'h',
      .lname = "help",
      .handle = handle_help,
      .reqv = false,
      .help = "shows the list of cli options",
      .unique = true
   },
   {
      .sname = 'g',
      .lname = "debug",
      .handle = handle_debug,
      .reqv = false,
      .help = "emits debug symbols",
      .unique = true
   },
   {
      .sname = 'k',
      .lname = "kawaii",
      .handle = handle_kawaii,
      .reqv = false,
      .help = "uses a kawaii tone",
      .unique = true
   },
   {
      .sname = 'v',
      .lname = "version",
      .handle = handle_version,
      .reqv = false,
      .help = "prints the current version",
      .unique = false
   },
   {
      .sname = 'S',
      .lname = "dump",
      .handle = handle_dump,
      .reqv = false,
      .help = "dumps intermediate representation",
      .unique = true
   },
   {
      .sname = 'O',
      .lname = "optimize",
      .handle = handle_optimize,
      .reqv = false,
      .help = "optimizes intermediate representation",
      .unique = true
   },
   {
      .sname = 'd',
      .lname = "direct",
      .handle = handle_direct,
      .reqv = false,
      .help = "generates .splo directly from AST",
      .unique = true
   },
   {
      .sname = '\0',
      .lname = "verbose",
      .handle = handle_verbose,
      .reqv = false,
      .help = "prints full program message",
      .unique = true
   },
   {
      .sname = 'n',
      .lname = "dry-run",
      .handle = handle_dryrun,
      .reqv = false,
      .help = "doesn't produce actual files",
      .unique = true
   },
   {
      .sname = 'j',
      .lname = "job",
      .handle = handle_job,
      .reqv = true,
      .help = "uses multi-thread",
      .unique = true
   },
   {
      .sname = 'o',
      .lname = "output",
      .handle = handle_output,
      .reqv = true,
      .help = "specifies the name of the output file",
      .unique = true
   },
   {
      .sname = '\0',
      .lname = "std",
      .handle = handle_standard,
      .reqv = true,
      .help = "specifies the language spec to use",
      .unique = true,
   },
   {
      .sname = '\0',
      .lname = "lang",
      .handle = handle_language,
      .reqv = true,
      .help = "specifies the system language to use",
      .unique = true
   },
   {
      .sname = '\0',
      .lname = "target",
      .handle = handle_target,
      .reqv = true,
      .help = "specifies the target langauge for transpiling",
      .unique = true
   },
   {
      .sname = '\0',
      .lname = "asm",
      .handle = handle_asmbly,
      .reqv = false,
      .help = "generates output from .splasm input file",
      .unique = true
   },
   {
      .sname = '\0',
      .lname = "disasm",
      .handle = handle_disasm,
      .reqv = false,
      .help = "generates .splasm from .splo",
      .unique = true
   },
   {
      .sname = '\0',
      .lname = "pedantic",
      .handle = handle_pedantic,
      .reqv = false,
      .help = "checks strictly. enables all -W flags",
      .unique = true
   },
   {
      .sname = 'W',
      .lname = "warn",
      .handle = handle_warning,
      .reqv = true,
      .help = "issues warnings for the specified criterion",
      .unique = false
   }
};
static const size_t optmds_len = ARRLEN(optmds);

/*==================*
 | EXTERN VARIABLES |
 *==================*/
config_t cfg = {0};

#endif
