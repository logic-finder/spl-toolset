#ifndef LOADFILE_INTERNALS_H
#define LOADFILE_INTERNALS_H

#include <ctype.h>

#include "fatal.h"
#include "wrappers.h"
#include "strutils.h"
#include "readline.h"

#define INIT_SIZE 128

static void destruct_line(void *item, size_t idx);

#endif
