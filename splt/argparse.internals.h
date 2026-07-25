#ifndef ARGPARSE_INTERNALS_H
#define ARGPARSE_INTERNALS_H

#include <string.h>
#include <stddef.h>
#include "fatal.h"
#include "global.h"
#include "wrapper.h"
#include "strutil.h"

typedef void arghandler_t(
   optflg_t *of,
   optval_t *ov,
   const char *arg
);

typedef struct opt {
   char *name;
   arghandler_t *handler;
} opt_t;

static void parse_endopt(optflg_t *of);
static void parse_filenm(optval_t *ov, const char *arg);
static void parse_shrtop(optflg_t *of, optval_t *ov, const char *arg);
static void parse_longop(optflg_t *of, optval_t *ov, const char *arg);

static arghandler_t handle_exeopt;
static arghandler_t handle_imdopt;
static arghandler_t handle_dscopt;
static arghandler_t handle_kwiopt;
static arghandler_t handle_lngopt;
static arghandler_t handle_retopt;
static arghandler_t handle_hlpopt;
static arghandler_t handle_vsnopt;

#endif
