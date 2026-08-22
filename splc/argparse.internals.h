#ifndef ARGPARSE_INTERNALS_H
#define ARGPARSE_INTERNALS_H

#include <string.h>
#include <stddef.h>

#include "fatal.h"
#include "common.h"
#include "global.h"
#include "wrappers.h"
#include "strutils.h"

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

static arghandler_t handle_vbsopt;
static arghandler_t handle_kwiopt;
static arghandler_t handle_lngopt;
static arghandler_t handle_outopt;
static arghandler_t handle_optopt;
static arghandler_t handle_dbgopt;
static arghandler_t handle_dmpopt;
static arghandler_t handle_tgtopt;
static arghandler_t handle_drnopt;
static arghandler_t handle_pdtopt;
static arghandler_t handle_wrnopt;
static arghandler_t handle_stdopt;
static arghandler_t handle_hlpopt;
static arghandler_t handle_vsnopt;

static const char *warnmsg =
   "  * note: should this be a filename, specify '" Cbwhite "--" Creset "' first\n"
   "  * note: type " Cbmagenta "--help" Creset " to see the manual page";

const char *stdopt_spl01 = "spl01";
const char *stdopt_cor27 = "cor27";
const char *stdopt = "std";
const char *outopt = "o";

#endif
