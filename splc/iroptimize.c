#include "iroptimize.h"
#include "iroptimize.internals.h"

extern void iroptimize(void) {
   fold_const(irt);
   // TODO: ASGN hearer const 뒤에 hearer의 값을 변경시킬 우려가 있는
   // 노드가 없는 상태에서 ASGN const hearer 조합이 나오면
   // SET const 실제값 으로 변경 -- 이건 예시IR을 써놓고 before after를 비교해서
   // 작성하는게 정확할듯
   // TODO: 캐릭터변수의 개입이 없는 연산자 축약
}

static void fold_const(tree_t *irt) {
   size_t irtsiz, actsiz, scenesiz;
   tree_t *act, *scene, *block, *new_block;

   /* irt -> act -> scene -> block -> opcode */

   /* k = 1 to skip SET dpsz 0 */
   irtsiz = tree_clen(irt);

   for (size_t k = 1; k < irtsiz; k++) {
      act = tree_child(irt, k);
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

            fold_const_work(block, new_block);

            tree_setchild(scene, n, new_block);
            /* since we reuse block->dat, need not to free it */
            tree_destroy_shallow(block);
         }
      }
   }
}

static void fold_const_work(tree_t *block, tree_t *new_block) {
   size_t blocksiz, j;
   tree_t *op, *p1, *p2, *temp;
   irnode_t *opdat, *p1dat, *p2dat, *tempdat;
   int val, flow_lnum, flow_lpos;
   bool flow_flag;

   blocksiz = tree_clen(block);

   for (size_t i = 0; i < blocksiz; i++) {
      /* Validates opcode */
      op = tree_child(block, i);
      opdat = tree_dat(op);
      if (opdat->dat.n != IropcodeSet)
         goto graft;

      /* Validates 1st parameter */
      p1 = tree_child(op, 0);
      p1dat = tree_dat(p1);
      if (p1dat->kind != IrnodekindVar)
         goto graft;
      if (p1dat->dat.n != IrvarConst)
         goto graft;

      /* Validates 2nd parameter */
      p2 = tree_child(op, 1);
      p2dat = tree_dat(p2);
      if (p2dat->kind != IrnodekindConst)
         goto graft;

      /* SET const (1 | -1) */
      val = p2dat->dat.n;
      flow_flag = false;

      /* Advances until temp is not 2x */
      for (j = i + 1; j < blocksiz; j++) {
         /* the next opcode must exist */
         temp = tree_child(block, j);
         tempdat = tree_dat(temp);
         if (tempdat->dat.n != Iropcode2x)
            break;
         if (val > SPL_INT_MAX / 2 || val < SPL_INT_MIN / 2)
            if (!flow_flag) {
               flow_flag = true;
               flow_lnum = tempdat->lnum;
               flow_lpos = tempdat->lpos;
               break;  /* stops before over/underflow */
            }
         val *= 2;
         tree_prune(temp);
      }
      if (flow_flag)
         warn(flow_lnum, flow_lpos);

      p2dat->dat.n = val;
      i = j - 1;  /* fast-forwards i */
   graft:
      tree_graft(new_block, op);
   }
}

static void warn(int lnum, int lpos) {
   line_t *l = arr_peek(ls, lnum - 1);
   fmtwrt(
      Cbred "\n<optimizer warning>" Creset " %s\n"
      "[%s:%d:%d] " Cbwhite "note:" Creset " problematic since here\n"
      "%4d|%.*s" Cbblue "%s" Creset "\n",
      msgs.warn.optimizer.flow,
      sfname, lnum, lpos,
      lnum, lpos - 1, l->run, &l->run[lpos - 1]
   );
}
