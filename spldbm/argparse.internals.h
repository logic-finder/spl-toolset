#ifndef ARGPARSE_INTERNALS_H
#define ARGPARSE_INTERNALS_H

#include <string.h>
#include "global.h"
#include "strutil.h"
#include "argparse.h"

typedef void parser_t(const char *arg);
typedef void arghandler_t(const char *arg);

typedef struct opt {
   char *name;
   arghandler_t *handler;
} opt_t;

static parser_t parse_shrtop;
static parser_t parse_longop;

static arghandler_t handle_makopt;
static arghandler_t handle_arcopt;
static arghandler_t handle_resopt;
static arghandler_t handle_kwiopt;
static arghandler_t handle_hlpopt;
static arghandler_t handle_vsnopt;

#endif
