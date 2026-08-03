#ifndef SPLRT_H
#define SPLRT_H

#include <stddef.h>
#include <stdbool.h>

#include "global.h"

#if (defined(__GNUC__) && (__GNUC__ > 5 || (__GNUC__ == 5 && __GNUC_MINOR__ >= 1))) \
   || (defined(__clang__) && (__clang__ > 3 || (__clang__ == 3 && __clang__ >= 8)))
#define HAS_ADD_OVERFLOW 1
#define HAS_SUB_OVERFLOW 1
#define HAS_MUL_OVERFLOW 1
#else
#define HAS_ADD_OVERFLOW 0
#define HAS_SUB_OVERFLOW 0
#define HAS_MUL_OVERFLOW 0
#endif

/************
 * TYPEDEFS *
 ************/
// fixme: 변수명 매크로화 후 codegen2c 등에서 사용
typedef struct {
   bool cond;
   spl_int_t *dp;  /* int [] */
   size_t dpsz;
   stack_t **mem;  /* stack_t * [] */
   stage_t *st;
   spl_uint_t
      t,  /* teller */
      h;  /* hearer */
   spl_int_t
      cnst,
      ol,
      or;
   stack_t *s;
} rt_ctx_t;

typedef spl_int_t operator_t(spl_int_t l);
typedef void iohandler_t(rt_ctx_t *rctx);

/**************
 * PROTOTYPES *
 **************/
rt_ctx_t *init_runtime(size_t dpsz);

operator_t op_sqrt;
operator_t op_squr;
operator_t op_cube;
operator_t op_fact;

iohandler_t io_inn;
iohandler_t io_inc;
iohandler_t io_outn;
iohandler_t io_outc;

void rememb(rt_ctx_t *rctx, spl_int_t v);
void recall(rt_ctx_t *rctx);

static void assert_offstage(rt_ctx_t *rctx, size_t charidx);

#endif
