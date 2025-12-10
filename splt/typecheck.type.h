#ifndef TYPECHECK_TYPE_H
#define TYPECHECK_TYPE_H

#include "db.h"
#include "msg.h"
#include "parse.h"
#include "global.h"
#include "wrapper.h"
#include "typecheck.h"
#include "colorcode.h"

static tree_callback_t coalesce_name;
static tree_callback_t typecheck_adj;
static tree_callback_t typecheck_noun;

static inline void semerr(void);
static teller_t tell;

#endif
