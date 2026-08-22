#ifndef OPTPROCESSOR_INTERNALS_H
#define OPTPROCESSOR_INTERNALS_H

#include <stdbool.h>

#include "fatal.h"
#include "global.h"
#include "colorcode.h"

static void validate_argc(int argc);
static void handle_pdtopt(compile_ctx_t *cctx);
static void default_stdopt(compile_ctx_t *cctx);

#endif
