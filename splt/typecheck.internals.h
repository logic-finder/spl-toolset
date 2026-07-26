#ifndef TYPECHECK_INTERNALS_H
#define TYPECHECK_INTERNALS_H

#include "msg.h"
#include "parse.h"
#include "global.h"
#include "wrappers.h"
#include "dbhandler.h"
#include "colorcode.h"

typedef void typechecker_t(node_t *n);

static void coalesce_title(void);
static void coalesce_name(tree_t *dp);
static void check_namecol(tree_t *dp);

static tree_callback_t typecheck_router;
static typechecker_t typecheck_name;
static typechecker_t typecheck_adj;
static typechecker_t typecheck_noun;
static typechecker_t typecheck_comp;
static typechecker_t typecheck_rnum;

static int is_pronoun(const char *s);
static bool is_rnum(const char *rnum);

static inline void print_errheader(void);
static void semerr_badword(node_t *n);
static void semerr_dupname(
   node_t * restrict curr,
   node_t * restrict prev
);

extern const char *sfname;  // see global.h
extern tree_t *pt;  // see global.h
extern msg_t msgs;  // see global.h

static const char *reason;  // error message

#endif
