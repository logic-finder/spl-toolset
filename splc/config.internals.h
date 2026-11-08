#ifndef CONFIG_INTERNALS_H
#define CONFIG_INTERNALS_H

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "global.h"
#include "config.h"
#include "wrappers.h"
#include "strutils.h"
#include "argp_drivers.h"

static bool print_manpage(compile_ctx_t *cctx);

static bool print_version(compile_ctx_t *cctx);

static inline bool easter_egg(compile_ctx_t *cctx);

static void panic_print_manpage(compile_ctx_t *cctx);

/*==================*
 | EXTERN VARIABLES |
 *==================*/
config_t cfg = {0};  /* Since config does not change, I made it external */

#endif
