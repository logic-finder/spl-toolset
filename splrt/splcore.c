#include "splcore.h"
#include "splcore.internals.h"

extern runtime_context_t *init_runtime(size_t dpsz) {
   runtime_context_t *rctx;

   rctx = safe_malloc(sizeof *rctx);

   rctx->cond = false;
   rctx->dp = safe_malloc(dpsz * ESIZ(rctx->dp));
   rctx->dpsz = dpsz;
   rctx->mem = init_mem(dpsz);
   rctx->st = stage_create(dpsz);

   return rctx;
}

extern void cleanup_runtime(runtime_context_t *rctx) {
   stage_destroy(rctx->st);
   cleanup_mem(rctx);
   free(rctx->dp);
}

extern spl_int_t op_sqrt(spl_int_t v) {
   if (v < 0)
      ERR("sqrt can't take a negative integer");
   return sqrt(v);
}

extern spl_int_t op_squr(spl_int_t v) {
   return v * v;
}

extern spl_int_t op_cube(spl_int_t v) {
   return v * v * v;
}

extern spl_int_t op_fact(spl_int_t v) {
   spl_int_t t;

   if (v < 0)
      ERR("factorial can't take a negative integer");

   if (v == 0)
      return 1;

   t = 1;
   while (v >= 2)
      t *= v--;

   return t;
}

extern void io_inn(runtime_context_t *rctx, size_t charidx) {
   io_in(rctx, charidx, "%" SPL_INT_FMTSPC);
}

extern void io_inc(runtime_context_t *rctx, size_t charidx) {
   io_in(rctx, charidx, "%c");
}

extern void io_outn(runtime_context_t *rctx, size_t charidx) {
   io_out(rctx, charidx, "%" SPL_INT_FMTSPC);
}

extern void io_outc(runtime_context_t *rctx, size_t charidx) {
   io_out(rctx, charidx, "%c");
}

static stack_t **init_mem(size_t dpsz) {
   stack_t **ret;

   ret = safe_malloc(dpsz * ESIZ(ret));  /* arr. of stack */
   for (size_t i = 0; i < dpsz; i++)
      ret[i] = stack_create();

   return ret;
}

static void cleanup_mem(runtime_context_t *rctx) {
   for (size_t i = 0; i < rctx->dpsz; i++)
      stack_destroy(rctx->mem[i]);
   free(rctx->mem);
}

static void io_in(
   runtime_context_t *rctx,
   size_t charidx,
   const char *fmt
) {
   if (charidx >= rctx->dpsz)
      VERR("%s", errmsg_charidx_oob);

   // fixme: write a function that counts the number of format specifiers in fmt
   safe_fscanf(stdin, fmt, 1, &rctx->dp[charidx]);
   clearbuf();
}

static void io_out(
   runtime_context_t *rctx,
   size_t charidx,
   const char *fmt
) {
   if (charidx >= rctx->dpsz)
      VERR("%s", errmsg_charidx_oob);

   safe_fprintf(stdout, fmt, 1, rctx->dp[charidx]);
}

static void clearbuf(void) {
   int ch;
   while ((ch = fgetc(stdin)) != EOF)
      /* empty */ ;
}

// fixme: meaningless?
static void assert_offstage(stage_t *st, size_t charidx) {
   if (stage_onstage(st, charidx))
      return;
   VERR(
      "a line by the offstage character \"%s\"",
      stage_name(st, charidx)
   );
}

// fixme: meaningless?
static void assert_onlytwo(stage_t *st) {
   if (stage_aretheretwo(st))
      return;
   VERR(
      "the number of onstage characters not 2; currently %d",
      stage_cnt(st)
   );
}
