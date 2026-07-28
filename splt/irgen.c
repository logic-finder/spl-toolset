#include "irgen.h"
#include "irgen.internals.h"

extern void irgenerate(void) {
   tree_t *nrtv;

   nrtv = tree_child(pt, 2);
   irt = plant_tree(NULL, 0, IrnodekindRoot, 0, 0);
   set_dpsz();
   tree_pre_traverse(nrtv, route, 0);
}

static void set_dpsz(void) {
   tree_t *dp, *data_sect;
   int dpsz;

   data_sect = graft_tree_n(
      irt,
      0,
      IrnodekindData,
      0,
      0
   );

   dp = tree_child(pt, 1);
   dpsz = tree_clen(dp);

   graft_tree_n(data_sect, IropcodeSet, IrnodekindInst, 0, 0);
   graft_tree_n(data_sect, IrvarDpsz, IrnodekindVar, 0, 0);
   graft_tree_n(data_sect, dpsz, IrnodekindData, 0, 0);
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
      case NODEKIND_ASGN1  : /* fall-through */
      case NODEKIND_ASGN2  : handle_asgn  (t); break;
      case NODEKIND_OUT_N  : handle_io(t, IropcodeOutN); break;
      case NODEKIND_OUT_C  : handle_io(t, IropcodeOutC); break;
      case NODEKIND_IN_N   : handle_io(t, IropcodeInN ); break;
      case NODEKIND_IN_C   : handle_io(t, IropcodeInC ); break;
      case NODEKIND_GOTO   : handle_goto  (t); break;
      case NODEKIND_COND   : handle_cond  (t); break;
      case NODEKIND_IF     : handle_if    (t); break;
      case NODEKIND_PUSH   : handle_push  (t); break;
      case NODEKIND_POP    : handle_pop   (t); break;
      default: ;  /* DEPEND, CONST, and operators fall here */
   }
}

static void handle_act(tree_t *t) {
   node_t *act, *romnum;

   act = tree_dat(t);
   romnum = tree_chdat(t, 0);

   curr_act = graft_tree_s(
      irt,
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
   romnum = tree_chdat(t, 0);

   curr_scene = graft_tree_s(
      curr_act,
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
   node_t *n;
   size_t len;
   int charidx;

   n = tree_dat(t);
   len = tree_clen(t);

   /* Generates 'ENTER <charidx>' */
   for (size_t i = 0; i < len; i++) {
      charidx = TREE_CHDAT(t, i)->dat.n;
      graft_tree_n(
         curr_block,
         IropcodeEnter,
         IrnodekindInst,
         n->lnum,
         n->lpos
      );
      graft_tree_n(
         curr_block,
         charidx,
         IrnodekindPerson,
         0,
         0
      );
   }
}

static void handle_exit(tree_t *t) {
   node_t *n;
   int charidx;

   n = tree_dat(t);
   charidx = TREE_CHDAT(t, 0)->dat.n;

   graft_tree_n(
      curr_block,
      IropcodeExit,
      IrnodekindInst,
      n->lnum,
      n->lpos
   );
   graft_tree_n(
      curr_block,
      charidx,
      IrnodekindPerson,
      0,
      0
   );
}

static void handle_exeunt(tree_t *t) {
   node_t *n;
   size_t len;
   int charidx;

   n = tree_dat(t);
   len = tree_clen(t);

   /* Generates 'EXEUNT' */
   if (!len) {
      graft_tree_n(
         curr_block,
         IropcodeExeunt,
         IrnodekindInst,
         n->lnum,
         n->lpos
      );
      return;
   }

   /* Generates 'ENTER <charidx>' */
   for (size_t i = 0; i < len; i++) {
      charidx = TREE_CHDAT(t, i)->dat.n;
      graft_tree_n(
         curr_block,
         IropcodeEnter,
         IrnodekindInst,
         n->lnum,
         n->lpos
      );
      graft_tree_n(
         curr_block,
         charidx,
         IrnodekindPerson,
         0,
         0
      );
   }
}

static void handle_line(tree_t *t) {
   node_t *dat, *chdat;
   int charidx;

   chdat = tree_chdat(t, 0);
   charidx = chdat->dat.n;
   dat = tree_dat(t);

   /* Generates 'SPEAK <charidx>' */
   graft_tree_n(
      curr_block,
      IropcodeSpeak,
      IrnodekindInst,
      dat->lnum,
      dat->lpos
   );
   graft_tree_n(
      curr_block,
      charidx,
      IrnodekindPerson,
      0,
      0
   );
}

static void handle_asgn(tree_t *t) {
   tree_t *c;
   node_t *n;

   c = tree_child(t, 0);
   resolve_const(c);  /* ... PUSH const */

   /* Generates 'POP hearer' */
   n = tree_dat(t);
   graft_tree_n(
      curr_block,
      IropcodePop,
      IrnodekindInst,
      n->lnum,
      n->lpos
   );
   graft_tree_n(curr_block, IrvarHearer, IrnodekindVar, 0, 0);
}

static void handle_io(tree_t *t, iropcode_t inst) {
   /* Generates '<inst>' */
   node_t *n = tree_dat(t);
   graft_tree_n(
      curr_block,
      inst,
      IrnodekindInst,
      n->lnum,
      n->lpos
   );
}

static void handle_goto(tree_t *t) {
   nodekind_t type;
   node_t *node, *child;

   node = tree_dat(t);
   type = node->dat.n;
   child = tree_chdat(t, 0);

   /* Note: NODEKIND value is used to determine
      where this GOTO heads -- Act or Scene */
   graft_tree_n(
      curr_block,
      IropcodeGoto,
      IrnodekindInst,
      node->lnum,
      node->lpos
   );
   graft_tree_n(
      curr_block,
      type,
      IrnodekindData,
      0,
      0
   );
   graft_tree_s(
      curr_block,
      child->dat.s.run,
      child->dat.s.len,
      IrnodekindData,
      0,
      0
   );
}

static void handle_cond(tree_t *t) {
   /* COND NODE STRUCTURE
         [0] = lhs -> p1 | p2 | (p3 -> const)
         [1] = negate | affirm
         [2] = eq | (ineq -> gt | lt)
         [3] = rhs -> const
      EXAMPLE
         !(lhs < rhs)
         lhs == rhs */

   tree_t *lhs_child, *rhs_child;
   node_t *node, *p, *op, *mode;
   irvar_t var;
   iropcode_t comp;

   /* Prepares left constant */
   lhs_child = tree_child(tree_child(t, 0), 0);
   p = tree_dat(lhs_child);

   if (p->kind == NODEKIND_P1 || p->kind == NODEKIND_P2) {
      graft_tree_n(
         curr_block,
         IropcodePush,
         IrnodekindInst,
         0,
         0
      );
      var = p->kind == NODEKIND_P1 ? IrvarTeller : IrvarHearer;
      graft_tree_n(
         curr_block,
         var,
         IrnodekindVar,
         0,
         0
      );
   }
   /* NODEKIND_P3 */
   else resolve_const(tree_child(lhs_child, 0));

   /* Prepares right constant */
   rhs_child = tree_child(tree_child(t, 3), 0);
   resolve_const(rhs_child);

   /* Determines ==, <, or > */
   op = tree_chdat(t, 2);
   switch (op->kind) {
      case NODEKIND_LT: comp = IropcodeLt; break;
      case NODEKIND_EQ: comp = IropcodeEqual; break;
      case NODEKIND_GT: comp = IropcodeGt; break;
      /* control never reaches here */
      default: comp = IropcodeUnknown;
   }

   /* Generates the comparison */
   node = tree_dat(t);
   set_operands();
   graft_tree_n(
      curr_block,
      comp,
      IrnodekindInst,
      node->lnum,
      node->lpos
   );
   graft_tree_n(
      curr_block,
      IrvarOperandL,
      IrnodekindVar,
      0,
      0
   );
   graft_tree_n(
      curr_block,
      IrvarOperandR,
      IrnodekindVar,
      0,
      0
   );

   /* Generates ! optionally */
   mode = tree_chdat(t, 1);
   if (mode->kind == NODEKIND_AFFIRM)
      return;  /* early return */
   graft_tree_n(
      curr_block,
      IropcodeNegate,
      IrnodekindInst,
      node->lnum,
      node->lpos
   );
}

static void handle_if(tree_t *t) {
   /* IF NODE STRUCTURE
      [0] AFFIRM | NEGATE
      [1] CONSEQ
         [0] statement */

   tree_t *conseq, *stmt;
   node_t *ifdat, *mode, *stmtdat;
   iropcode_t jump;
   size_t my_cnt;

   /* Generates
         JUMPTRUE  .Ln  if  "if not, ..."
         JUMPFALSE .Ln  if  "if so , ..."
      where n = labelcnt + 1 */

   mode = tree_chdat(t, 0);
   switch (mode->kind) {
      case NODEKIND_AFFIRM: jump = IropcodeJumpF; break;
      case NODEKIND_NEGATE: jump = IropcodeJumpT; break;
      /* control never reaches here */
      default: jump = IropcodeUnknown;
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
   graft_tree_n(curr_block, my_cnt, IrnodekindData, 0, 0);

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
      case NODEKIND_OUT_N  : handle_io(t, IropcodeOutN); break;
      case NODEKIND_OUT_C  : handle_io(t, IropcodeOutC); break;
      case NODEKIND_IN_N   : handle_io(t, IropcodeInN ); break;
      case NODEKIND_IN_C   : handle_io(t, IropcodeInC ); break;
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
   tree_t *c;
   node_t *node;
   irnode_t *irnode;
   size_t len, instidx;

   c = tree_child(t, 0);
   resolve_const(c);  /* ... PUSH const */

   /* Replaces PUSH with <inst> */
   len = tree_clen(curr_block);
   instidx = len - 2;

   node = tree_dat(t);
   irnode = tree_chdat(curr_block, instidx);
   irnode->dat.n = IropcodeRememb;
   irnode->lnum = node->lnum;
   irnode->lpos = node->lpos;
}

static void handle_pop(tree_t *t) {
   node_t *n = tree_dat(t);
   graft_tree_n(
      curr_block,
      IropcodeRecall,
      IrnodekindInst,
      n->lnum,
      n->lpos
   );
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
      case NODEKIND_SUM  : resolve_binary_op(child, IropcodeSum ); return;
      case NODEKIND_DIFF : resolve_binary_op(child, IropcodeDiff); return;
      case NODEKIND_PROD : resolve_binary_op(child, IropcodeProd); return;
      case NODEKIND_QUOT : resolve_binary_op(child, IropcodeQuot); return;
      case NODEKIND_REM  : resolve_binary_op(child, IropcodeRem ); return;
      case NODEKIND_SQRT : resolve_unary_op(child, IropcodeSqrt); return;
      case NODEKIND_SQUR : resolve_unary_op(child, IropcodeSqur); return;
      case NODEKIND_CUBE : resolve_unary_op(child, IropcodeCube); return;
      case NODEKIND_2X   : resolve_unary_op(child, Iropcode2x  ); return;
      case NODEKIND_FACT : resolve_unary_op(child, IropcodeFact); return;
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
         Iropcode2x,
         IrnodekindInst,
         adjdat->lnum,
         adjdat->lpos
      );
      graft_tree_n(curr_block, IrvarConst, IrnodekindVar, 0, 0);
   }

   /* Generates 'PUSH const' */
   graft_tree_n(curr_block, IropcodePush, IrnodekindInst, 0, 0);
   graft_tree_n(curr_block, IrvarConst, IrnodekindVar , 0, 0);
}

static void resolve_unary_op(tree_t *t, iropcode_t inst) {
   tree_t *c;
   node_t *op;

   /* operator -> const */
   c = tree_child(t, 0);
   resolve_const(c);

   /* Generates 'POP operand_left */
   graft_tree_n(curr_block, IropcodePop, IrnodekindInst, 0, 0);
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

static void resolve_binary_op(tree_t *t, iropcode_t inst) {
   tree_t *lc, *rc;
   node_t *op;

   /* operator -> lhs / rhs -> const */
   lc = tree_child(tree_child(t, 0), 0);
   rc = tree_child(tree_child(t, 1), 0);

   resolve_const(lc);
   resolve_const(rc);

   /* Generates
         POP operand_right
         POP operand_left */
   set_operands();

   /* Generates '<inst> const left right' */
   op = tree_dat(t);
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
      IropcodeSet,
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

      default:  /* control never reaches here */
         val = -1;
         kind = IrnodekindUnknown;
   }
   graft_tree_n(curr_block, val, kind, 0, 0);
}

static void set_operands(void) {
   graft_tree_n(curr_block, IropcodePop, IrnodekindInst, 0, 0);
   graft_tree_n(curr_block, IrvarOperandR, IrnodekindVar, 0, 0);
   graft_tree_n(curr_block, IropcodePop, IrnodekindInst, 0, 0);
   graft_tree_n(curr_block, IrvarOperandL, IrnodekindVar, 0, 0);
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

   node.datkind = IrnodeDatkindStr;
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

   node.datkind = IrnodeDatkindInt;
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
