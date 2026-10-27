#ifndef ARGPARSE_INTERNALS_H
#define ARGPARSE_INTERNALS_H

#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "fatal.h"
#include "common.h"
#include "argparse.h"
#include "strutils.h"
#include "array.adt.h"
#include "arena.adt.h"
#include "colorcodes.h"

typedef struct {
   argtok_t tok;
   argerr_t err;
   void *ctx;
} errinfo_t;

typedef struct {
   arena_t *arena;
   int argc;
   const char **argv;
   const char *arg;
   array_t *toks;
   argtok_t *tok;
   size_t toks_len;
   size_t idx;
   bool eoe;
   optmd_t *table;
   size_t table_len;
   array_t *sourcefiles;
   size_t errcnt;
   size_t errcnt_max;
   argerr_t err;
   array_t *err_infos;
   bool lookahead_ret;
   bool preprocess_ret;
} argp_ctx_t;

typedef struct {
   optmd_t *table;
   size_t table_len;
} errhandle_unrecognized_ctx_t;

typedef enum [[nodiscard]] {
   ArgpStatOk,
   ArgpStatErrcntFull,
   ArgpStatErr
} argp_stat_t;

[[nodiscard]]
static argp_stat_t validate_schemas(argp_ctx_t *actx);

[[nodiscard]]
static argp_stat_t tokenize_argv(argp_ctx_t *actx);

[[nodiscard]]
static argp_stat_t parse_argtoks(argp_ctx_t *actx);

[[nodiscard]]
static bool handle_posarg(argp_ctx_t *actx);

[[nodiscard]]
static bool populate_as_dashdash(argp_ctx_t *actx);

[[nodiscard]]
static bool populate_as_longflag(argp_ctx_t *actx);

[[nodiscard]]
static bool populate_as_shortflag(argp_ctx_t *actx);

[[nodiscard]]
static bool populate_as_posarg(argp_ctx_t *actx);

[[nodiscard]]
static optmd_t *query_by_shortname(argp_ctx_t *actx, char flag);

[[nodiscard]]
static optmd_t *query_by_longname(argp_ctx_t *actx, const char *flag);

[[nodiscard]]
static argp_stat_t gettok(argp_ctx_t *actx);

static void ungettok(argp_ctx_t *actx);

[[nodiscard]]
static argp_stat_t preprocess_shortflag(argp_ctx_t *actx);

[[nodiscard]]
static argp_stat_t preprocess_longflag(argp_ctx_t *actx);

[[nodiscard]]
static argp_stat_t lookahead(argp_ctx_t *actx);

static array_destructor_t destruct_argtok;

static array_destructor_t destruct_errinfo;

[[nodiscard]]
static opt_errhandler_t errhandle_seen_multiple;

[[nodiscard]]
static opt_errhandler_t errhandle_unrecognized;

[[nodiscard]]
static opt_errhandler_t errhandle_out_of_tok;

[[nodiscard]]
static argp_stat_t regerr(argp_ctx_t *actx, argtok_t *tok);

[[nodiscard]]
static bool report_errors(argp_ctx_t *actx);

static void showargtoks(argp_ctx_t *actx, bool terse);

static void showargtok(void *dat, size_t idx, void *ctx);

static void showargtok_terse(void *dat, size_t idx, void *ctx);

static const char *resolve_argtok_kind(argtok_t *tok);

static void list_options(argp_ctx_t *actx);

#endif
