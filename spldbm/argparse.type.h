#ifndef ARGPARSE_TYPE_H
#define ARGPARSE_TYPE_H

#include <string.h>
#include "global.h"
#include "strutil.h"
#include "argparse.h"

typedef void arghandler_t(const char *arg);

typedef struct opt {
   char *name;
   arghandler_t *handler;
} opt_t;

static void parse_shrtop(const char *arg);
static void parse_longop(const char *arg);

static arghandler_t handle_hlpopt;
static arghandler_t handle_vsnopt;
static arghandler_t handle_makopt;
static arghandler_t handle_arcopt;
static arghandler_t handle_resopt;
static arghandler_t handle_kwiopt;

#endif
