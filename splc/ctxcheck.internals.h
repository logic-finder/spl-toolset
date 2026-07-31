#ifndef CTXCHECK_INTERNALS_H
#define CTXCHECK_INTERNALS_H

#include <stdbool.h>
#include "msg.h"
#include "global.h"
#include "wrappers.h"

typedef void ctxchecker_t(tree_t *t);

static tree_callback_t ctxcheck_router;
static inline ctxchecker_t ctxcheck_act;
static inline ctxchecker_t ctxcheck_scene;
static ctxchecker_t ctxcheck_goto;

static void print_err(node_t *n);
static void trace(tree_t *t, const char *type);
static void semerr_unknown_label(node_t *n, const char *s);

extern const char *sfname;  // see global.h
extern msg_t msgs;          // see global.h

static const char *reason;  // error message
static tree_t *nrtv, *act, *scene;

#endif
