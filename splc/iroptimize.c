#include "iroptimize.h"
#include "iroptimize.internals.h"

/* OPTIMIZATION IDEAS
   - Remove dead assigns
      : 앞선 대입을 덮어쓰는 대입문이 있을시, 최종적인 대입문만 남기기
         you a rose!
         you a house!
      위 코드에서, 첫번째 대입문에 의해 값이 rose로 설정됐지만 즉시 house로 대체되므로
      첫번째 대입문을 삭제해도 you의 값에는 변화가 없음

   - Reduce trivial operators
      : 연산자에서, 두 피연산자가 상수로만 이루어져 있다면 연산자 제거하고 직접 계산하기
         you are as good as the sum of 3 and 5!
      3과 5는 상수이므로 컴파일타임에 그 값이 이미 분명하므로
         you are 8!
      이렇게 바꾸어도 you의 값에는 변화가 없음

   - Propagate constants
      : 변수의 값을 컴파일타임에 확정할 수 있다면 해당 변수를 상수로 대체
         you a cookie!
         you are the sum of 3 and yourself!
      두 대입문 사이에 you의 값을 예측할 수 없게 하는 요소(예: 조건문+대입문)가 없으므로
         you are the sum of 3 and 1!
      이렇게 치환 가능할 것으로 생각됨

   - Reduce if statements
      : 같은 조건을 가지는 연속된 if문은 하나로 줄이기
         if so, if so, ...
      ~이면서 ~인 것은 항상 참이므로 if so 하나로 줄여도 무방

   - Remove contradictions
      : 모순되는 if문 쌍을 제거
         if so, if not, ...
      ~이면서 ~가 아닌 것은 항상 거짓이므로 ...가 실행될 일이 없음

   - Remove trivial comparisons
      : 비교의 두 대상을 모두 확정할 수 있다면 컴파일타임에 비교
         Am I less confusing than me? -> x < x 이므로 무조건 거짓
         Are you better than yourself? -> x > x 이므로 무조건 거짓
         Am I as good as me? -> x == x 이므로 무조건 사실
      변수에 사용되는 경우 명확한 경우는 위 3가지 밖에 없는 듯함.
      조건 변수를 나타내는 변수를 Irvar에 추가하고, 관련 변경사항을 ir관련 소스코드에 전부 반영할 것
      한편, x == x 꼴과 달리 x != 2 * x 와 같은 것은 x가 0일 경우 거짓이 되므로 제거 불가
         Is a tree better than a tree? -> 1 > 1
         Is a tree worse than a tree? -> 1 < 1
         Is a tree as good as a tree? -> 1 == 1
         Is a tree not as good as a shiny tree? -> 1 != 2
      상수로만 이루어져 있을 경우 위 4가지 경우에만 명백한 듯

   - Remove trivial operators
      : * 1 이나 + 0 은 의미가 없으므로 제거

   - (2 + a) + 3 == a + 5 이런식으로 바꿀수도 있을듯 (나눗셈은 예외케이스 있을듯)
*/

extern void iroptimize(compile_ctx_t *cctx) {
   iroptmiz_ctx_t zctx;

   zctx.ls = cctx->ls;
   zctx.ov = cctx->ov;
   zctx.irt = cctx->irt;
   zctx.nrtv = tree_child(zctx.irt, 1);

   safe_fputs(stdout, ENPREFIX "optimizing IR...");

   /* Note: the order fc -> pc -> ro & rc is intended */
   fold_const(&zctx);
   // propagate_const()
   // reduce_operator()
   // remove_comparison()
   // remove_deadasgn()
   // reduce_ifstmt()
   // remove_contradict()

   safe_vprintf(" " Cgreen "done!" Creset
         "\t(total " Cbwhite "%zu" Creset " nodes)\n",
         count_opcodes(cctx->irt)
      );
}

static void fold_const(iroptmiz_ctx_t *zctx) {
   size_t nrtvsiz, actsiz, scenesiz;
   tree_t *act, *scene, *block, *new_block;

   /* irt -> [1] nrtv -> act -> scene -> block -> opcode */

   // fixme: no longer meaningful?
   // /* k = 1 to skip SET dpsz 0 */

   nrtvsiz = tree_clen(zctx->nrtv);

   for (size_t k = 0; k < nrtvsiz; k++) {
      act = tree_child(zctx->nrtv, k);
      actsiz = tree_clen(act);

      for (size_t m = 0; m < actsiz; m++) {
         scene = tree_child(act, m);
         scenesiz = tree_clen(scene);

         for (size_t n = 0; n < scenesiz; n++) {
            block = tree_child(scene, n);
            new_block = tree_plant(NULL, 0);

            tree_setparent(new_block, tree_parent(block));
            /* reuse block->dat */
            tree_setdat(new_block, tree_dat(block));

            fold_const_work(zctx, block, new_block);

            tree_setchild(scene, n, new_block);
            /* since we reuse block->dat, need not to free it */
            tree_destroy_shallow(block);
         }
      }
   }
}

static void fold_const_work(iroptmiz_ctx_t *zctx, tree_t *block, tree_t *new_block) {
   size_t blocksiz, j;
   tree_t *op, *p1, *p2, *temp;
   irnode_t *opdat, *p1dat, *p2dat, *tempdat;
   int val;
   size_t flow_lnum, flow_lpos;
   bool flow_flag;

   blocksiz = tree_clen(block);

   for (size_t i = 0; i < blocksiz; i++) {
      /* Validates opcode */
      op = tree_child(block, i);
      opdat = tree_dat(op);
      if (opdat->dat.i != IropcodeSet)
         goto graft;

      /* Validates 1st parameter */
      p1 = tree_child(op, 0);
      p1dat = tree_dat(p1);
      if (p1dat->kind != IrnodekindVar)
         goto graft;
      if (p1dat->dat.i != IrvarConst)
         goto graft;

      /* Validates 2nd parameter */
      p2 = tree_child(op, 1);
      p2dat = tree_dat(p2);
      if (p2dat->kind != IrnodekindConst)
         goto graft;

      /* SET const (1 | -1) */
      val = p2dat->dat.i;
      flow_flag = false;

      /* Advances until temp is not 2x */
      for (j = i + 1; j < blocksiz; j++) {
         /* the next opcode must exist */
         temp = tree_child(block, j);
         tempdat = tree_dat(temp);
         if (tempdat->dat.i != Iropcode2x)
            break;
         if (val > SPL_INT_MAX / 2 || val < SPL_INT_MIN / 2) {
            flow_flag = true;
            flow_lnum = tempdat->lnum;
            flow_lpos = tempdat->lpos;
            break;  /* stops before over/underflow */
         }
         val *= 2;
         tree_prune(temp);
      }
      if (flow_flag)
         warn(zctx, flow_lnum, flow_lpos);

      p2dat->dat.i = val;
      i = j - 1;  /* fast-forwards i */

   graft:
      tree_graft(new_block, op);
   }
}

static void warn(iroptmiz_ctx_t *zctx, size_t lnum, size_t lpos) {
   line_t *l;

   l = array_peek(zctx->ls, lnum - 1);

   safe_vprintf(
      Cbred "\n<optimizer warning>" Creset " %s\n"
      "[%s:%zu:%zu] " Cbwhite "note:" Creset " problematic since here\n"
      "%4d|%.*s" Cbblue "%s" Creset "\n",
      msgs.warn.optimizer.flow,
      zctx->ov->src, lnum, lpos,
      lnum, lpos - 1, l->run, &l->run[lpos - 1]
   );
}
