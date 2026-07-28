#include "irgen.h"
#include "irgen.internals.h"

static tree_t *irroot, *curr_act, *curr_scene, *curr_block;
static int labelcnt;

extern void irgenerate(void) {
   tree_t *nrtv;

   nrtv = tree_child(pt, 2);
   irroot = plant_tree(NULL, 0, IrnodekindRoot, 0, 0);
   tree_pre_traverse(nrtv, route, 0);
}

static void route(tree_t *t, int lv) {
   node_t *n;

   (void) lv;
   n = tree_dat(t);

   switch (n->kind) {
      case NODEKIND_ACT    : handle_act   (t); break;
      case NODEKIND_SCENE  : handle_scene (t); break;
      case NODEKIND_ENTER  : handle_enter (t); break;
      case NODEKIND_EXIT   : handle_exit  (t); break;
      case NODEKIND_EXEUNT : handle_exeunt(t); break;
      case NODEKIND_LINE   : handle_line  (t); break;
      case NODEKIND_ASGN1  : handle_asgn  (t); break;
      case NODEKIND_ASGN2  : handle_asgn  (t); break;
      case NODEKIND_OUT_N  : handle_outn  (t); break;
      case NODEKIND_OUT_C  : handle_outc  (t); break;
      case NODEKIND_IN_N   : handle_inn   (t); break;
      case NODEKIND_IN_C   : handle_inc   (t); break;
      case NODEKIND_GOTO   : handle_goto  (t); break;
      case NODEKIND_COND   : handle_cond  (t); break;
      case NODEKIND_IF     : handle_if    (t); break;
      case NODEKIND_PUSH   : handle_push  (t); break;
      case NODEKIND_POP    : handle_pop   (t); break;
      default: ;  /* control CAN reach here */
   }
}

static void handle_act(tree_t *t) {
   node_t *act, *romnum;

   act = tree_dat(t);
   romnum = tree_chdat(act, 0);

   curr_act = graft_tree_s(
      irroot,
      romnum->dat.s.run,
      romnum->dat.s.len,
      IrnodekindAct,
      act->lnum,
      act->lpos
   );
}

static void handle_scene(tree_t *t) {
   node_t *scene, *romnum;

   scene = tree_dat(t);
   romnum = tree_chdat(scene, 0);

   curr_scene = graft_tree_s(
      irroot,
      romnum->dat.s.run,
      romnum->dat.s.len,
      IrnodekindScene,
      scene->lnum,
      scene->lpos
   );
   labelcnt = 0;
   add_block(labelcnt);  /* updates curr_block */
}

static void handle_enter(tree_t *t) {

}

static void handle_exit(tree_t *t) {

}

static void handle_exeunt(tree_t *t) {

}

static void handle_line(tree_t *t) {

}

static void handle_asgn(tree_t *t) {
   resolve_const();
   // pop
   // hearer
}

static void handle_outn(tree_t *t) {

}

static void handle_outc(tree_t *t) {

}

static void handle_inn(tree_t *t) {

}

static void handle_inc(tree_t *t) {

}

static void handle_goto(tree_t *t) {

}

static void handle_cond(tree_t *t) {

}

static void handle_if(tree_t *t) {
   /* IF NODE STRUCTURE
      [0] AFFIRM | NEGATE
      [1] CONSEQ
         [0] statement */

   tree_t *conseq, *stmt;
   node_t *ifdat, *mode, *stmtdat;
   irinst_t jump;
   size_t my_cnt;

   /* Generates
         JUMPTRUE  .Ln  if  "if not, ..."
         JUMPFALSE .Ln  if  "if so , ..."
      where n = labelcnt + 1 */

   mode = tree_chdat(t, 0);
   switch (mode->kind) {
      case NODEKIND_AFFIRM: jump = IrinstJumpF; break;
      case NODEKIND_NEGATE: jump = IrinstJumpT; break;
      default: ;  /* control never reaches here */
   }
   ifdat = tree_dat(t);
   graft_tree_n(
      curr_block,
      jump,
      IrnodekindInst,
      ifdat->lnum,
      ifdat->lpos
   );
   my_cnt = ++labelcnt;  /* labelcnt updated */
   graft_tree_n(curr_block, my_cnt, IrnodekindLabelNum, 0, 0);

   /* Example IR - "if not, recall your shiny proud!"
      .L1:
         JUMPTRUE .L2
         RECALL

      .L2:
         ... */

   conseq = tree_child(t, 1);
   stmt = tree_child(conseq, 0);
   stmtdat = tree_dat(stmt);
   switch (stmtdat->kind) {
      case NODEKIND_ASGN1  : /* fall-through */
      case NODEKIND_ASGN2  : handle_asgn(stmt); break;
      case NODEKIND_OUT_N  : handle_outn(stmt); break;
      case NODEKIND_OUT_C  : handle_outc(stmt); break;
      case NODEKIND_IN_N   : handle_inn (stmt); break;
      case NODEKIND_IN_C   : handle_inc (stmt); break;
      case NODEKIND_GOTO   : handle_goto(stmt); break;
      case NODEKIND_COND   : handle_cond(stmt); break;
      case NODEKIND_PUSH   : handle_push(stmt); break;
      case NODEKIND_POP    : handle_pop (stmt); break;
      case NODEKIND_IF     : handle_if  (stmt); break;
      default: ;  /* control never reaches here */
   }

   add_block(my_cnt);  /* updates curr_block */

   /* Marks 'stmt' with DEPEND so as to prevent it from
      getting caught in 'route()' */
   stmtdat->kind = NODEKIND__DEPEND;
}

static void handle_push(tree_t *t) {

}

static void handle_pop(tree_t *t) {

}

static void resolve_const(tree_t *t) {
   /* CONST NODE STRUCTURE
      const -> [adj...] (noun | char)
      const -> op -> (const | const const) */

   tree_t *child, *noun;
   node_t *chdat, *adjdat;
   size_t clen;

   child = tree_child(t, 0);
   chdat = tree_dat(child);

   /* Checks if this tree is an operator */
   switch (chdat->kind) {
      case NODEKIND_SUM  : resolve_binary_op(child, IrinstSum ); return;
      case NODEKIND_DIFF : resolve_binary_op(child, IrinstDiff); return;
      case NODEKIND_PROD : resolve_binary_op(child, IrinstProd); return;
      case NODEKIND_QUOT : resolve_binary_op(child, IrinstQuot); return;
      case NODEKIND_REM  : resolve_binary_op(child, IrinstRem ); return;
      case NODEKIND_SQRT : resolve_unary_op(child, IrinstSqrt); return;
      case NODEKIND_SQUR : resolve_unary_op(child, IrinstSqur); return;
      case NODEKIND_CUBE : resolve_unary_op(child, IrinstCube); return;
      case NODEKIND_2X   : resolve_unary_op(child, Irinst2x  ); return;
      case NODEKIND_FACT : resolve_unary_op(child, IrinstFact); return;
      default: ;  /* not an operator */
   }

   clen = tree_clen(t);
   noun = tree_child(t, clen - 1);

   /* Generates 'SET const (1 | -1)' */
   resolve_noun(noun);

   /* Generates '2X const' */
   for (size_t i = 0; i < clen - 1; i++) {  /* only adjs */
      adjdat = tree_chdat(t, i);
      graft_tree_n(
         curr_block,
         Irinst2x,
         IrnodekindInst,
         adjdat->lnum,
         adjdat->lpos
      );
      graft_tree_n(curr_block, IrvarConst, IrnodekindVar, 0, 0);
   }

   /* Generates 'PUSH const' */
   graft_tree_n(curr_block, IrinstPush, IrnodekindInst, 0, 0);
   graft_tree_n(curr_block, IrvarConst, IrnodekindVar , 0, 0);
}

static void resolve_unary_op(tree_t *t, irinst_t inst) {
   tree_t *c;
   node_t *op;

   c = tree_child(tree_child(t, 0), 0);
   resolve_const(c);

   /* Generates 'POP operand_left */
   graft_tree_n(curr_block, IrinstPop, IrnodekindInst, 0, 0);
   graft_tree_n(curr_block, IrvarOperandL, IrnodekindVar, 0, 0);

   op = tree_dat(t);

   /* Generates '<inst> const left */
   graft_tree_n(
      curr_block,
      inst,
      IrnodekindInst,
      op->lnum,
      op->lpos
   );
   graft_tree_n(curr_block, IrvarConst, IrnodekindVar, 0, 0);
   graft_tree_n(curr_block, IrvarOperandL, IrnodekindVar, 0, 0);
}

static void resolve_binary_op(tree_t *t, irinst_t inst) {
   tree_t *lc, *rc;
   node_t *op;

   lc = tree_child(tree_child(t, 0), 0);
   rc = tree_child(tree_child(t, 1), 0);

   resolve_const(lc);
   resolve_const(rc);

   /* Generates
         POP operand_right
         POP operand_left */

   graft_tree_n(curr_block, IrinstPop, IrnodekindInst, 0, 0);
   graft_tree_n(curr_block, IrvarOperandR, IrnodekindVar, 0, 0);
   graft_tree_n(curr_block, IrinstPop, IrnodekindInst, 0, 0);
   graft_tree_n(curr_block, IrvarOperandL, IrnodekindVar, 0, 0);

   op = tree_dat(t);

   /* Generates '<inst> const left right' */
   graft_tree_n(
      curr_block,
      inst,
      IrnodekindInst,
      op->lnum,
      op->lpos
   );
   graft_tree_n(curr_block, IrvarConst, IrnodekindVar, 0, 0);
   graft_tree_n(curr_block, IrvarOperandL, IrnodekindVar, 0, 0);
   graft_tree_n(curr_block, IrvarOperandR, IrnodekindVar, 0, 0);
}

static void resolve_noun(tree_t *t) {
   node_t *node;
   int val;
   irnodekind_t kind;

   /* Generates 'SET const (1 | -1)' */
   node = tree_dat(t);

   graft_tree_n(
      curr_block,
      IrinstSet,
      IrnodekindInst,
      node->lnum,
      node->lpos
   );

   graft_tree_n(curr_block, IrvarConst, IrnodekindVar, 0, 0);

   switch (node->kind) {
      case NODEKIND_PNOUN:
         val  = 1;
         kind = IrnodekindConst;
      break;

      case NODEKIND_NNOUN:
         val  = -1;
         kind = IrnodekindConst;
      break;

      case NODEKIND_P1:
         val  = IrvarTeller;
         kind = IrnodekindVar;
      break;

      case NODEKIND_P2:
         val  = IrvarHearer;
         kind = IrnodekindVar;
      break;

      case NODEKIND_CHAR:
         val  = node->dat.n;
         kind = IrnodekindPerson;
      break;

      default: ;  /* control never reaches here */
   }
   graft_tree_n(curr_block, val, kind, 0, 0);
}

static tree_t *plant_tree(
   const char *run,
   size_t len,
   irnodekind_t kind,
   int lnum,
   int lpos
) {
   irnode_t node;
   char *buf;

   if (run) {
      buf = smalloc(len);  /* already has a room for \0 */
      memcpy(buf, run, len);
   }
   else buf = NULL;

   node.datkind = DATKIND_STR;
   node.dat.s.run = buf;
   node.dat.s.len = len;
   node.kind = kind;
   node.lnum = lnum;
   node.lpos = lpos;

   return tree_plant(&node, sizeof node);
}

static tree_t *graft_tree_s(
   tree_t *base,
   const char *run,
   size_t len,
   irnodekind_t kind,
   size_t lnum,
   size_t lpos
) {
   tree_t *sub = plant_tree(run, len, kind, lnum, lpos);
   return tree_graft(base, sub);
}

static tree_t *graft_tree_n(
   tree_t *base,
   int val,
   irnodekind_t kind,
   size_t lnum,
   size_t lpos
) {
   irnode_t node;
   tree_t *sub;

   node.datkind = DATKIND_INT;
   node.dat.n = val;
   node.kind = kind;
   node.lnum = lnum;
   node.lpos = lpos;
   sub = tree_plant(&node, sizeof node);

   return tree_graft(base, sub);
}

static void add_block(int n) {
   curr_block = graft_tree_n(
      curr_scene,
      n,
      IrnodekindBlock,
      0,
      0
   );
}
