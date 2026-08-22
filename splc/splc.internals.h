#ifndef SPLC_H
#define SPLC_H

#include <ctype.h>
#include <string.h>

#include "msg.h"
#include "lex.h"
#include "irgen.h"
#include "parse.h"
#include "irdump.h"
#include "global.h"
#include "wrappers.h"
#include "loadfile.h"
#include "ctxcheck.h"
#include "dbhandler.h"
#include "transpile.h"
#include "typecheck.h"
#include "colorcode.h"
#include "assembler.h"
#include "codegen2c.h"
#include "iroptimize.h"
#include "optprocessor.h"

/* Miscellaneous */
static array_iterator_t print_token;
static tree_callback_t print_node;
static const char *nodekind2str(nodekind_t kind);

#endif
