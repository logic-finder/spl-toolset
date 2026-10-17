#ifndef CONFIG_INTERNALS_H
#define CONFIG_INTERNALS_H

#include <stdio.h>
#include <stdlib.h>

#include "fatal.h"
#include "global.h"
#include "config.h"
#include "wrappers.h"
#include "strutils.h"
#include "argp_drivers.h"

static void print_manpage(void);
static void print_version(void);
static inline void easter_egg(void);

/*==================*
 | EXTERN VARIABLES |
 *==================*/
config_t cfg = {0};  /* Since config does not change, I made it external */

#endif
