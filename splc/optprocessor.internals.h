#ifndef OPTPROCESSOR_INTERNALS_H
#define OPTPROCESSOR_INTERNALS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "fatal.h"
#include "global.h"
#include "wrappers.h"
#include "colorcodes.h"

static void validate_argc(int argc);
static void handle_hlpopt(compile_ctx_t *cctx);
static void handle_vsnopt(compile_ctx_t *cctx);
static void handle_pdtopt(compile_ctx_t *cctx);
static void default_stdopt(compile_ctx_t *cctx);
static void default_outopt(compile_ctx_t *cctx);

#endif
