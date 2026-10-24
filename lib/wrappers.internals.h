#ifndef WRAPPERS_INTERNALS_H
#define WRAPPERS_INTERNALS_H

#include <errno.h>

#include "fatal.h"
#include "wrappers.h"

static void safe_malloc_cb_default(int errcode, size_t sz);

static safe_malloc_cb_t *safe_malloc_callback = safe_malloc_cb_default;

#endif
