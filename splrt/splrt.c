#include "splrt.h"
#include "splrt.internals.h"

extern rt_ctx_t *init_runtime(size_t dpsz) {
   rt_ctx_t *rctx;

   rctx = safe_malloc(sizeof *rctx);

   rctx->cond = false;
   rctx->dp = safe_malloc(dpsz * ESIZ(rctx->dp));
   rctx->dpsz = dpsz;
   rctx->mem = init_mem(dpsz);
   rctx->st = stage_create(dpsz);
   rctx->s = stack_create();

   return rctx;
}

extern void cleanup_runtime(rt_ctx_t *rctx) {
   stage_destroy(rctx->st);
   cleanup_mem(rctx);
   free(rctx->dp);
}

extern spl_int_t op_sum(spl_int_t l, spl_int_t r) {
#if HAS_ADD_OVERFLOW
   spl_int_t ret;

   if (__builtin_add_overflow(l, r, &ret))
      VERR("%s: over-or-underflow detected: %" SPL_INT_FMTSPC " + %" SPL_INT_FMTSPC,
            __func__, l, r);

   return ret;
#else
   if (l > 0 && r > 0) {
      if (SPL_INT_MAX - l < r)
         VERR("%s: overflow detected: %" SPL_INT_FMTSPC " + %" SPL_INT_FMTSPC,
            __func__, l, r);
   }
   else
   if (l < 0 && r < 0) {
      if (SPL_INT_MIN - l > r)
         VERR("%s: underflow detected: %" SPL_INT_FMTSPC " + %" SPL_INT_FMTSPC,
            __func__, l, r);
   }

   return l + r;
#endif
}

extern spl_int_t op_diff(spl_int_t l, spl_int_t r) {
#if HAS_SUB_OVERFLOW
   spl_int_t ret;

   if (__builtin_sub_overflow(l, r, &ret))
      VERR("%s: over-or-underflow detected: %" SPL_INT_FMTSPC " - %" SPL_INT_FMTSPC,
            __func__, l, r);

   return ret;

#else
   if (l > 0 && r < 0) {
      if (l > SPL_INT_MAX + r)
         VERR("%s: overflow detected: %" SPL_INT_FMTSPC " - %" SPL_INT_FMTSPC,
            __func__, l, r);
   }
   else
   if (l < 0 && r > 0) {
      if (l < SPL_INT_MIN + r)
         VERR("%s: underflow detected: %" SPL_INT_FMTSPC " - %" SPL_INT_FMTSPC,
            __func__, l, r);
   }

   return l - r;
#endif
}

extern spl_int_t op_prod(spl_int_t l, spl_int_t r) {
#if HAS_MUL_OVERFLOW
   spl_int_t ret;

   if(__builtin_mul_overflow(l, r, &ret))
      VERR("%s: over-or-underflow detected: %" SPL_INT_FMTSPC " * %" SPL_INT_FMTSPC,
            __func__, l, r);

   return ret;
#else
   spl_uint_t ul, ur, max;

   if (l == 0 || r == 0)
      return 0;

   ul = l < 0 ? - (spl_uint_t) l : l;
   ur = r < 0 ? - (spl_uint_t) r : r;
   max = (l ^ r) < 0 ? - (spl_uint_t) SPL_INT_MIN : SPL_INT_MAX;

   if (ul > max / ur)
      VERR("%s: over-or-underflow detected: %" SPL_INT_FMTSPC " * %" SPL_INT_FMTSPC,
            __func__, l, r);

   return l * r;
#endif
}

extern spl_int_t op_quot(spl_int_t l, spl_int_t r) {
   if (r == 0)
      VERR("%s: division by 0 detected!", __func__);

   if (l == SPL_INT_MIN && r == -1)
      VERR("%s: overflow detected: MIN / -1", __func__);

   return l / r;
}

extern spl_int_t op_rem(spl_int_t l, spl_int_t r) {
   if (r == 0)
      VERR("%s: division by 0 detected!", __func__);

   if (l == SPL_INT_MIN && r == -1)
      VERR("%s: overflow detected: MIN % -1", __func__);

   return l % r;
}

extern spl_int_t op_sqrt(spl_int_t l) {
   if (l < 0)
      ERR("sqrt can't take a negative integer");
   return sqrt(l);
}

extern spl_int_t op_squr(spl_int_t l) {
   return op_prod(l, l);
}

extern spl_int_t op_cube(spl_int_t l) {
   return op_prod(op_prod(l, l), l);
}

extern spl_int_t op_2x(spl_int_t l) {
   if (l > SPL_INT_MAX / 2 || l < SPL_INT_MIN / 2)
      VERR("%s: over-or-underflow detected", __func__);

   return 2 * l;
}

extern spl_int_t op_fact(spl_int_t l) {
   spl_int_t t;

   if (l < 0)
      ERR("factorial can't take a negative integer");

   if (l == 0)
      return 1;

   t = 1;
   while (l >= 2)
      t *= l--;

   return t;
}

extern void io_inn(rt_ctx_t *rctx) {
   io_in(rctx, "%" SPL_INT_FMTSPC);
}

extern void io_inc(rt_ctx_t *rctx) {
   io_in(rctx, "%c");
}

extern void io_outn(rt_ctx_t *rctx) {
   io_out(rctx, "%" SPL_INT_FMTSPC);
}

extern void io_outc(rt_ctx_t *rctx) {
   io_out(rctx, "%c");
}

extern void rememb(rt_ctx_t *rctx, spl_int_t v) {
   set_hearer(rctx);
   stack_push(rctx->mem[rctx->h], v);
}

extern void recall(rt_ctx_t *rctx) {
   set_hearer(rctx);
   stack_pop(rctx->mem[rctx->h], &rctx->dp[rctx->h]);
}

extern void assert_offstage(rt_ctx_t *rctx, size_t charidx) {
   if (stage_onstage(rctx->st, charidx))
      return;
   VERR(
      "an offstage character \"%s\" used",
      stage_name(rctx->st, charidx)
   );
}

extern void set_hearer(rt_ctx_t *rctx) {
   assert_onlytwo(rctx);
   rctx->h = stage_whoareyou(rctx->st, rctx->t);
}

static stack_t **init_mem(size_t dpsz) {
   stack_t **ret;

   ret = safe_malloc(dpsz * ESIZ(ret));  /* stack_t * [] */
   for (size_t i = 0; i < dpsz; i++)
      ret[i] = stack_create();

   return ret;
}

static void cleanup_mem(rt_ctx_t *rctx) {
   for (size_t i = 0; i < rctx->dpsz; i++)
      stack_destroy(rctx->mem[i]);
   free(rctx->mem);
}

static void io_in(
   rt_ctx_t *rctx,
   const char *fmt
) {
   set_hearer(rctx);
   // fixme: write a function that counts the number of format specifiers in fmt
   safe_fscanf(stdin, fmt, 1, &rctx->dp[rctx->h]);
   clearbuf();
}

static void io_out(
   rt_ctx_t *rctx,
   const char *fmt
) {
   set_hearer(rctx);
   safe_fprintf(stdout, fmt, 1, rctx->dp[rctx->h]);
}

static void clearbuf(void) {
   int ch;
   while ((ch = fgetc(stdin)) != EOF)
      /* empty */ ;
}

static void assert_onlytwo(rt_ctx_t *rctx) {
   if (stage_aretheretwo(rctx->st))
      return;
   VERR(
      "the number of onstage characters not 2; currently %d",
      stage_cnt(rctx->st)
   );
}
