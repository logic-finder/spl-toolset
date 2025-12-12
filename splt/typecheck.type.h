#ifndef TYPECHECK_TYPE_H
#define TYPECHECK_TYPE_H

#include "db.h"
#include "msg.h"
#include "parse.h"
#include "global.h"
#include "wrapper.h"
#include "typecheck.h"
#include "colorcode.h"

typedef void typechecker_t(node_t *n);

static tree_callback_t coalesce_name;
static tree_callback_t typecheck_router;

static typechecker_t typecheck_adj;
static typechecker_t typecheck_noun;
static typechecker_t typecheck_char;
static typechecker_t typecheck_comp;
static typechecker_t typecheck_rnum;

static bool validate_name(const char *s, int *ret);
static bool validate_pronoun(const char *s);

static void mark_whom(char *tag, int num);
static bool is_valid_rnum(const char *rnum);

static inline void semerr(void);
static teller_t tell;

#endif
