#ifndef ARGPARSE_INTERNALS_H
#define ARGPARSE_INTERNALS_H

#include <string.h>
#include "global.h"

typedef void parser_t(const char *arg);
typedef void arghandler_t(const char *arg);

typedef struct opt {
   char *name;
   arghandler_t *handler;
} opt_t;

static void parse_endopt(void);
static parser_t parse_filenm;
static parser_t parse_shrtop;
static parser_t parse_longop;

static arghandler_t handle_nwnopt;
static arghandler_t handle_kwiopt;
static arghandler_t handle_hlpopt;
static arghandler_t handle_vsnopt;

/**********************
 * External Variables *
 **********************/
optflg_t of = {0};
optval_t ov;

#endif
