#ifndef ARGPARSE_INTERNALS_H
#define ARGPARSE_INTERNALS_H

#include <setjmp.h>
#include <string.h>
#include <stdlib.h>

#include "fatal.h"
#include "common.h"
#include "argparse.h"
#include "wrappers.h"
#include "strutils.h"
#include "array.adt.h"
#include "colorcodes.h"

typedef struct {
   argtok_t tok;
   argerr_t err;
} errinfo_t;

typedef struct {
   int argc;
   const char **argv;
   const char *arg;
   array_t *toks;
   argtok_t *tok;
   size_t toks_len;
   size_t idx;
   bool eoe;
   volatile size_t errcnt;
   size_t errcnt_max;
   optmd_t *table;
   size_t table_len;
   array_t *sourcefiles;
   jmp_buf env;
   argerr_t err;
   array_t *err_infos;
} argp_ctx_t;

static void validate_schemas(argp_ctx_t *actx);
static void tokenize_argv(argp_ctx_t *actx);
static void parse_argtoks(argp_ctx_t *actx);

static void handle_posarg(argp_ctx_t *actx);

static void populate_as_dashdash(argp_ctx_t *actx);
static void populate_as_longflag(argp_ctx_t *actx);
static void populate_as_shortflag(argp_ctx_t *actx);
static void populate_as_posarg(argp_ctx_t *actx);

static optmd_t *query_by_shortname(argp_ctx_t *actx, char flag);
static optmd_t *query_by_longname(argp_ctx_t *actx, const char *flag);

static argtok_t *gettok(argp_ctx_t *actx);
static void ungettok(argp_ctx_t *actx);

static bool preprocess_shortflag(argp_ctx_t *actx);
static bool preprocess_longflag(argp_ctx_t *actx);
static bool lookahead(argp_ctx_t *actx);

static array_destructor_t destruct_argtok;

static void errhandle_seen_multiple(argtok_t *tok, const char *reason);
static void errhandle_unrecognized(argtok_t *tok, const char *reason);
static void errhandle_out_of_tok(argtok_t *tok, const char *reason);
static void regerr(argp_ctx_t *actx, argtok_t *tok);

static void report_errors(argp_ctx_t *actx);

static void showargtoks(argp_ctx_t *actx, bool terse);
static void showargtok(void *dat, size_t idx, void *ctx);
static void showargtok_terse(void *dat, size_t idx, void *ctx);
static const char *resolve_argtok_kind(argtok_t *tok);

static void list_options(argp_ctx_t *actx);

#endif
