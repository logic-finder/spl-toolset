#ifndef CTXCHECK_TYPE_H
#define CTXCHECK_TYPE_H

#include <stdbool.h>
#include "msg.h"
#include "global.h"
#include "wrapper.h"
#include "ctxcheck.h"

typedef struct trace {
   bool onstage;
   node_t *loc;
} trace_t;

typedef void ctxchecker_t(tree_t *t);

static void init_traces(void);
static tree_callback_t ctxcheck_router;

static void aretheretwo(
   const char * restrict solerr,
   const char * restrict referr
);
static inline bool isonstage(int who);
static int whoareyou(int me);

static void ctxcheck_stack(
   tree_t *t,
   const char * restrict solerr,
   const char * restrict referr
);
static void ctxcheck_p(tree_t *t, int p);
static ctxchecker_t ctxcheck_act;
static ctxchecker_t ctxcheck_enter;
static ctxchecker_t ctxcheck_exit;
static ctxchecker_t ctxcheck_exeunt;
static ctxchecker_t ctxcheck_line;
static ctxchecker_t ctxcheck_asgn;
static ctxchecker_t ctxcheck_goto;
static ctxchecker_t ctxcheck_cond;
static ctxchecker_t ctxcheck_push;
static ctxchecker_t ctxcheck_pop;
static ctxchecker_t ctxcheck_p1;
static ctxchecker_t ctxcheck_p2;
static ctxchecker_t ctxcheck_scene;

static inline void header_err(void);
static inline void header_warn(void);
static void errtrace(void);

static void semerr_exeunt(void);
static void semerr_line(const char *s);
static void semerr_goto(const char *s);
static void semerr_scene(void);
static void semerr_onstagecnt(void);
static void semwarn_stage(const char *s);

static void graft_tree(
   tree_t *base,
   int num,
   nodekind_t kind
);

#endif
