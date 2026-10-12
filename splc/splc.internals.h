#ifndef SPLC_INTERNALS_H
#define SPLC_INTERNALS_H

#include <ctype.h>
#include <string.h>

#include "msg.h"
#include "lex.h"
#include "irgen.h"
#include "parse.h"
#include "config.h"
#include "irdump.h"
#include "global.h"
#include "argparse.h"
#include "wrappers.h"
#include "loadfile.h"
#include "ctxcheck.h"
#include "dbhandler.h"
#include "transpile.h"
#include "typecheck.h"
#include "colorcodes.h"
#include "assembler.h"
#include "codegen2c.h"
#include "iroptimize.h"
#include "argp_drivers.h"

static void compile(config_t *cfg);
static void loadfile_talkative(compile_ctx_t *cctx, const char *filename);
static void checkif_argc1(int argc, optmd_t table[], size_t table_len);

#endif
