#include "irgen.h"
#include "irgen.internals.h"

extern void irgenerate(compile_ctx_t *cctx) {
   irgen_ctx_t ictx;

   /* irt -> [0] dp
          -> [1] nrtv */

   ictx.pt_dp = tree_child(cctx->pt, 1);
   ictx.pt_nrtv = tree_child(cctx->pt, 2);

   if (cctx->of->vbs) safe_fputs(stdout, ENPREFIX "generating IR...");

   ictx.irt = plant_tree(NULL, 0, IrnodekindRoot, 0, 0);
   set_dpsz(&ictx);  /* appends the dp tree to irt */
   ictx.nrtv = graft_tree_i(ictx.irt, 0, IrnodekindNrtv);
   tree_pre_traverse(ictx.pt_nrtv, route, 0, &ictx);
   /* Marks the last node with END OF PROGRAM */
   graft_tree_i(ictx.curr_block, 0, IrnodekindEop);

   if (cctx->of->vbs) safe_vprintf(" " Cgreen "done!" Creset
      "\t(total " Cbwhite "%zu" Creset " nodes)\n",
      count_opcodes(ictx.irt)
   );

   cctx->irt = ictx.irt;
   return;
}

extern void destroy_irt(compile_ctx_t *cctx) {
   tree_post_traverse(cctx->irt, cleanup_irnode, 0, NULL);
   tree_prune(cctx->irt);
}

static void cleanup_irnode(tree_t *t, int lv, void *ctx) {
   irnode_t *n;

   (void) lv, (void) ctx;

   n = tree_dat(t);

   if (n->datkind != IrnodeDatkindStr) {
      return;
   }

   free(n->dat.s.run);
}

static void set_dpsz(irgen_ctx_t *ictx) {
   tree_t *opcode;
   int dpsz;

   ictx->dp = graft_tree_i(ictx->irt, 0, IrnodekindDp);
   dpsz = (int) tree_clen(ictx->pt_dp);

   opcode = graft_tree_opcode(ictx->dp, IropcodeSet, 0, 0);
   graft_tree_ui(opcode, IrvarDpsz, IrnodekindVar);
   graft_tree_i(opcode, dpsz, IrnodekindConst);
}

static void route(tree_t *t, int lv, void *ctx) {
   irgen_ctx_t *ictx;

   (void) lv;

   ictx = ctx;
   ictx->t = t;
   ictx->n = tree_dat(t);

   switch (ictx->n->kind) {
      case NODEKIND_ACT    : handle_act   (ictx); break;
      case NODEKIND_SCENE  : handle_scene (ictx); break;
      case NODEKIND_ENTER  : handle_enter (ictx); break;
      case NODEKIND_EXIT   : handle_exit  (ictx); break;
      case NODEKIND_EXEUNT : handle_exeunt(ictx); break;
      case NODEKIND_LINE   : handle_line  (ictx); break;
      case NODEKIND_ASGN1  : /* fall-through */
      case NODEKIND_ASGN2  : /* fall-through */
      case NODEKIND_ASGN3  : handle_asgn  (ictx); break;
      case NODEKIND_OUT_N  : handle_io(ictx, IropcodeOutN); break;
      case NODEKIND_OUT_C  : handle_io(ictx, IropcodeOutC); break;
      case NODEKIND_IN_N   : handle_io(ictx, IropcodeInN ); break;
      case NODEKIND_IN_C   : handle_io(ictx, IropcodeInC ); break;
      case NODEKIND_GOTO   : handle_goto  (ictx); break;
      case NODEKIND_COND   : handle_cond  (ictx); break;
      case NODEKIND_IF     : handle_if    (ictx); break;
      case NODEKIND_PUSH   : handle_push  (ictx); break;
      case NODEKIND_POP    : handle_pop   (ictx); break;
      default: ;  /* DEPEND, CONST, and operators fall here */
   }
}

static void handle_act(irgen_ctx_t *ictx) {
   node_t *romnum;

   romnum = tree_chdat(ictx->t, 0);

   ictx->curr_act = graft_tree_s(
      ictx->nrtv,
      romnum->dat.s.run,
      romnum->dat.s.len,
      IrnodekindAct,
      ictx->n->lnum,
      ictx->n->lpos
   );
}

static void handle_scene(irgen_ctx_t *ictx) {
   node_t *romnum;

   romnum = tree_chdat(ictx->t, 0);

   ictx->curr_scene = graft_tree_s(
      ictx->curr_act,
      romnum->dat.s.run,
      romnum->dat.s.len,
      IrnodekindScene,
      ictx->n->lnum,
      ictx->n->lpos
   );
   ictx->labelcnt = 0;
   add_block(ictx, ictx->labelcnt);  /* updates curr_block */
}

static void handle_enter(irgen_ctx_t *ictx) {
   tree_t *opcode;
   size_t len;
   node_t *chdat;
   spl_uint_t charidx;

   len = tree_clen(ictx->t);

   /* Generates 'ENTER <charidx>' */
   for (size_t i = 0; i < len; i++) {
      chdat = tree_chdat(ictx->t, i);
      charidx = chdat->dat.n;
      opcode = graft_tree_opcode(
         ictx->curr_block,
         IropcodeEnter,
         ictx->n->lnum,
         ictx->n->lpos
      );
      graft_tree_charidx(opcode, charidx);
   }
}

static void handle_exit(irgen_ctx_t *ictx) {
   tree_t *opcode;
   node_t *chdat;
   spl_uint_t charidx;

   chdat = tree_chdat(ictx->t, 0);
   charidx = chdat->dat.n;

   opcode = graft_tree_opcode(
      ictx->curr_block,
      IropcodeExit,
      ictx->n->lnum,
      ictx->n->lpos
   );
   graft_tree_charidx(opcode, charidx);
}

static void handle_exeunt(irgen_ctx_t *ictx) {
   tree_t *opcode;
   size_t len;
   node_t *chdat;
   spl_uint_t charidx;

   len = tree_clen(ictx->t);

   /* Generates 'EXEUNT' */
   if (!len) {
      graft_tree_opcode(
         ictx->curr_block,
         IropcodeExeunt,
         ictx->n->lnum,
         ictx->n->lpos
      );
      return;
   }

   /* Generates 'EXIT <charidx>' */
   for (size_t i = 0; i < len; i++) {
      chdat = tree_chdat(ictx->t, i);
      charidx = chdat->dat.n;
      opcode = graft_tree_opcode(
         ictx->curr_block,
         IropcodeExit,
         ictx->n->lnum,
         ictx->n->lpos
      );
      graft_tree_charidx(opcode, charidx);
   }
}

static void handle_line(irgen_ctx_t *ictx) {
   tree_t *opcode;
   node_t *dat, *chdat;
   spl_uint_t charidx;

   chdat = tree_chdat(ictx->t, 0);
   charidx = chdat->dat.n;
   dat = tree_dat(ictx->t);

   /* Generates 'SPEAK <charidx>' */
   opcode = graft_tree_opcode(
      ictx->curr_block,
      IropcodeSpeak,
      dat->lnum,
      dat->lpos
   );
   graft_tree_charidx(opcode, charidx);
}

static void handle_asgn(irgen_ctx_t *ictx) {
   tree_t *c, *opcode;

   c = tree_child(ictx->t, 0);
   resolve_const(c, ictx);  /* ... PUSH const */

   // fixme: POP을 생성하지 말고, PUSH const를 ASGN hearer const로 고쳐도 되지 않을까?

   /* Generates 'POP hearer' */
   opcode = graft_tree_opcode(
      ictx->curr_block,
      IropcodePop,
      ictx->n->lnum,
      ictx->n->lpos
   );
   graft_tree_var(opcode, IrvarHearer);
}

static void handle_io(irgen_ctx_t *ictx, iropcode_t opcode) {
   /* Generates '<opcode>' */
   graft_tree_opcode(
      ictx->curr_block,
      opcode,
      ictx->n->lnum,
      ictx->n->lpos
   );
}

static void handle_goto(irgen_ctx_t *ictx) {
   tree_t *opcode;
   nodekind_t type;
   node_t *child;

   type = ictx->n->dat.n;
   child = tree_chdat(ictx->t, 0);

   opcode = graft_tree_opcode(
      ictx->curr_block,
      IropcodeGoto,
      ictx->n->lnum,
      ictx->n->lpos
   );
   graft_tree_ui(opcode, type, IrnodekindData);
   graft_tree_s(
      opcode,
      child->dat.s.run,
      child->dat.s.len,
      IrnodekindData,
      0,
      0
   );
}

static void handle_cond(irgen_ctx_t *ictx) {
   /* COND NODE STRUCTURE
         [0] = lhs -> p1 | p2 | (p3 -> const)
         [1] = negate | affirm
         [2] = eq | (ineq -> gt | lt)
         [3] = rhs -> const */

   tree_t *lhs_child, *rhs_child, *opcode;
   node_t *p, *op, *mode;
   irvar_t var;
   iropcode_t comp;

   /* Prepares left constant */
   lhs_child = tree_child(tree_child(ictx->t, 0), 0);
   p = tree_dat(lhs_child);

   if (p->kind == NODEKIND_P1 || p->kind == NODEKIND_P2) {
      opcode = graft_tree_opcode(
         ictx->curr_block,
         IropcodePush,
         0,
         0
      );
      var = p->kind == NODEKIND_P1 ? IrvarTeller : IrvarHearer;
      graft_tree_var(opcode, var);
   }
   /* NODEKIND_P3 */
   else resolve_const(tree_child(lhs_child, 0), ictx);

   /* Prepares right constant */
   rhs_child = tree_child(tree_child(ictx->t, 3), 0);
   resolve_const(rhs_child, ictx);

   /* Determines ==, <, or > */
   op = tree_chdat(ictx->t, 2);
   switch (op->kind) {
      case NODEKIND_LT: comp = IropcodeLt; break;
      case NODEKIND_EQ: comp = IropcodeEq; break;
      case NODEKIND_GT: comp = IropcodeGt; break;
      /* control never reaches here */
      default: comp = Iropcode_Unknown;
   }

   /* Generates the comparison */
   set_operands(ictx);
   opcode = graft_tree_opcode(
      ictx->curr_block,
      comp,
      ictx->n->lnum,
      ictx->n->lpos
   );
   graft_tree_var(opcode, IrvarOperandL);
   graft_tree_var(opcode, IrvarOperandR);

   /* Generates ! optionally */
   mode = tree_chdat(ictx->t, 1);
   if (mode->kind == NODEKIND_AFFIRM)
      return;  /* early return */
   graft_tree_opcode(
      ictx->curr_block,
      IropcodeNegate,
      ictx->n->lnum,
      ictx->n->lpos
   );
}

static void handle_if(irgen_ctx_t *ictx) {
   /* IF NODE STRUCTURE
      [0] AFFIRM | NEGATE
      [1] CONSEQ
         [0] statement */

   tree_t *conseq, *stmt, *opcode;
   node_t *mode, *stmtdat;
   iropcode_t jump;
   size_t my_cnt;

   /* Generates
         JUMPTRUE  .Ln  if  "if not, ..."
         JUMPFALSE .Ln  if  "if so , ..."
      where n = labelcnt + 1 */

   mode = tree_chdat(ictx->t, 0);
   switch (mode->kind) {
      case NODEKIND_AFFIRM: jump = IropcodeJumpF; break;
      case NODEKIND_NEGATE: jump = IropcodeJumpT; break;
      /* control never reaches here */
      default: jump = Iropcode_Unknown;
   }
   opcode = graft_tree_opcode(
      ictx->curr_block,
      jump,
      ictx->n->lnum,
      ictx->n->lpos
   );
   my_cnt = ++ictx->labelcnt;  /* labelcnt updated */
   graft_tree_ui(opcode, my_cnt, IrnodekindData);

   /* Example IR - "if not, recall your shiny proud!"
      .L1:
         JUMPTRUE .L2
         RECALL

      .L2:
         ... */

   conseq = tree_child(ictx->t, 1);
   stmt = tree_child(conseq, 0);
   stmtdat = tree_dat(stmt);

   ictx->t = stmt;
   ictx->n = stmtdat;

   switch (stmtdat->kind) {
      case NODEKIND_ASGN1  : /* fall-through */
      case NODEKIND_ASGN2  : /* fall-through */
      case NODEKIND_ASGN3  : handle_asgn(ictx); break;
      case NODEKIND_OUT_N  : handle_io(ictx, IropcodeOutN); break;
      case NODEKIND_OUT_C  : handle_io(ictx, IropcodeOutC); break;
      case NODEKIND_IN_N   : handle_io(ictx, IropcodeInN ); break;
      case NODEKIND_IN_C   : handle_io(ictx, IropcodeInC ); break;
      case NODEKIND_GOTO   : handle_goto(ictx); break;
      case NODEKIND_COND   : handle_cond(ictx); break;
      case NODEKIND_PUSH   : handle_push(ictx); break;
      case NODEKIND_POP    : handle_pop (ictx); break;
      case NODEKIND_IF     : handle_if  (ictx); break;
      default: ;  /* control never reaches default */
   }

   add_block(ictx, my_cnt);  /* updates curr_block */

   /* Marks 'stmt' with DEPEND so as to prevent it from
      getting caught in 'route()' */
   stmtdat->kind = NODEKIND__DEPEND;
}

/* Note: this "push" means the Remember statement;
      not to be confused with IropcodePush! */
static void handle_push(irgen_ctx_t *ictx) {
   tree_t *c;
   irnode_t *irnode;
   size_t last;

   c = tree_child(ictx->t, 0);
   resolve_const(c, ictx);  /* ... PUSH const */

   /* Replaces PUSH with REMEMB */
   last = tree_clen(ictx->curr_block) - 1;

   irnode = tree_chdat(ictx->curr_block, last);
   irnode->dat.ui = IropcodeRememb;
   irnode->lnum = ictx->n->lnum;
   irnode->lpos = ictx->n->lpos;
}

static void handle_pop(irgen_ctx_t *ictx) {
   graft_tree_opcode(
      ictx->curr_block,
      IropcodeRecall,
      ictx->n->lnum,
      ictx->n->lpos
   );
}

// fixme: 두번째인자로 PUSH const를 생성할지 말지 결정하기
// 너무 complex해질거같긴 함 <- 아이디어만 주석으로 남겨놓는게 좋을듯
static void resolve_const(tree_t *t, irgen_ctx_t *ictx) {
   /* CONST NODE STRUCTURE
      const -> [adj...] (noun | char)
      const -> op -> (const | const const) */

   tree_t *child, *noun, *opcode;
   node_t *chdat, *adjdat;
   size_t clen;

   child = tree_child(t, 0);
   chdat = tree_dat(child);

   /* Checks if this tree is an operator */
   switch (chdat->kind) {
      case NODEKIND_SUM  : resolve_binary_op(child, ictx, IropcodeSum ); return;
      case NODEKIND_DIFF : resolve_binary_op(child, ictx, IropcodeDiff); return;
      case NODEKIND_PROD : resolve_binary_op(child, ictx, IropcodeProd); return;
      case NODEKIND_QUOT : resolve_binary_op(child, ictx, IropcodeQuot); return;
      case NODEKIND_REM  : resolve_binary_op(child, ictx, IropcodeRem ); return;
      case NODEKIND_SQRT : resolve_unary_op(child, ictx, IropcodeSqrt); return;
      case NODEKIND_SQUR : resolve_unary_op(child, ictx, IropcodeSqur); return;
      case NODEKIND_CUBE : resolve_unary_op(child, ictx, IropcodeCube); return;
      case NODEKIND_2X   : resolve_unary_op(child, ictx, Iropcode2x  ); return;
      case NODEKIND_FACT : resolve_unary_op(child, ictx, IropcodeFact); return;
      default: ;  /* not an operator */
   }

   clen = tree_clen(t);
   noun = tree_child(t, clen - 1);

   /* Generates
         SET const 1|-1
      or
         ASGN const teller|hearer|dp[n]' */
   resolve_noun(noun, ictx);

   /* Generates '2x const const' */
   for (size_t i = 0; i < clen - 1; i++) {  /* only adjs */
      adjdat = tree_chdat(t, i);
      opcode = graft_tree_opcode(
         ictx->curr_block,
         Iropcode2x,
         adjdat->lnum,
         adjdat->lpos
      );
      graft_tree_var(opcode, IrvarConst);
      graft_tree_var(opcode, IrvarConst);
   }

   /* Generates 'PUSH const' */
   opcode = graft_tree_opcode(
      ictx->curr_block, IropcodePush, 0, 0);
   graft_tree_var(opcode, IrvarConst);
}

static void resolve_unary_op(tree_t *t, irgen_ctx_t *ictx, iropcode_t opcode) {
   tree_t *c, *opcode_tree;
   node_t *op;

   /* operator -> const */
   c = tree_child(t, 0);
   resolve_const(c, ictx);

   /* Generates 'POP operand_left */
   opcode_tree = graft_tree_opcode(
      ictx->curr_block, IropcodePop, 0, 0);
   graft_tree_var(opcode_tree, IrvarOperandL);

   op = tree_dat(t);

   /* Generates '<opcode> const left */
   opcode_tree = graft_tree_opcode(
      ictx->curr_block,
      opcode,
      op->lnum,
      op->lpos
   );
   graft_tree_var(opcode_tree, IrvarConst);
   graft_tree_var(opcode_tree, IrvarOperandL);

   /* Generates 'PUSH const' */
   opcode_tree = graft_tree_opcode(
      ictx->curr_block, IropcodePush, 0, 0);
   graft_tree_var(opcode_tree, IrvarConst);
}

static void resolve_binary_op(tree_t *t, irgen_ctx_t *ictx, iropcode_t opcode) {
   tree_t *lc, *rc, *opcode_tree;
   node_t *op;

   /* operator -> lhs / rhs -> const */
   lc = tree_child(tree_child(t, 0), 0);
   rc = tree_child(tree_child(t, 1), 0);

   resolve_const(lc, ictx);
   resolve_const(rc, ictx);

   /* Generates
         POP operand_right
         POP operand_left */
   set_operands(ictx);

   /* Generates '<opcode> const left right' */
   op = tree_dat(t);
   opcode_tree = graft_tree_opcode(
      ictx->curr_block,
      opcode,
      op->lnum,
      op->lpos
   );
   graft_tree_var(opcode_tree, IrvarConst);
   graft_tree_var(opcode_tree, IrvarOperandL);
   graft_tree_var(opcode_tree, IrvarOperandR);

   /* Generates 'PUSH const' */
   opcode_tree = graft_tree_opcode(
      ictx->curr_block, IropcodePush, 0, 0);
   graft_tree_var(opcode_tree, IrvarConst);
}

static void resolve_noun(tree_t *t, irgen_ctx_t *ictx) {
   tree_t *opcode;
   node_t *node;
   int val;
   irnodekind_t nodekind;
   iropcode_t opkind;

   /* Generates
         SET const (1 | -1)
      or
         ASGN (teller | hearer | dp[n]) */
   node = tree_dat(t);

   /* SET is to assign a number, while ASGN a variable */
   switch (node->kind) {
      case NODEKIND_ZERO:
         val = 0;
         nodekind = IrnodekindConst;
         opkind = IropcodeSet;
      break;

      case NODEKIND_PNOUN:
         val = 1;
         nodekind = IrnodekindConst;
         opkind = IropcodeSet;
      break;

      case NODEKIND_NNOUN:
         val = -1;
         nodekind = IrnodekindConst;
         opkind = IropcodeSet;
      break;

      case NODEKIND_P1:
         val = IrvarTeller;
         nodekind = IrnodekindVar;
         opkind = IropcodeAsgn;
      break;

      case NODEKIND_P2:
         val = IrvarHearer;
         nodekind = IrnodekindVar;
         opkind = IropcodeAsgn;
      break;

      case NODEKIND_CHAR:
         val = node->dat.n + Irvar_Dp_Begin;
         nodekind = IrnodekindPerson;
         opkind = IropcodeAsgn;
      break;

      default:  /* control never reaches here */
         val = -1;
         nodekind = Irnodekind_Unknown;
   }

   opcode = graft_tree_opcode(
      ictx->curr_block,
      opkind,
      node->lnum,
      node->lpos
   );

   graft_tree_var(opcode, IrvarConst);

   switch (node->kind) {
      case NODEKIND_ZERO : /* fall-through */
      case NODEKIND_PNOUN: /* fall-through */
      case NODEKIND_NNOUN:
         graft_tree_i(opcode, val, nodekind);
      break;

      case NODEKIND_P1: /* fall-through */
      case NODEKIND_P2:
         graft_tree_var(opcode, val);
      break;

      case NODEKIND_CHAR:
         graft_tree_charidx(opcode, val);
      break;

      default: ;
   }
}

static void set_operands(irgen_ctx_t *ictx) {
   tree_t *opcode;

   opcode = graft_tree_opcode(
      ictx->curr_block, IropcodePop, 0, 0);
   graft_tree_var(opcode, IrvarOperandR);

   opcode = graft_tree_opcode(
      ictx->curr_block, IropcodePop, 0, 0);
   graft_tree_var(opcode, IrvarOperandL);
}

static tree_t *plant_tree(
   const char *run,
   size_t len,
   irnodekind_t kind,
   size_t lnum,
   size_t lpos
) {
   irnode_t node;
   char *buf;

   if (run) {
      buf = safe_malloc(len + 1);  /* +1 for \0 */
      memcpy(buf, run, len + 1);  /* \0 copied */
   }
   else buf = NULL;

   node.datkind = IrnodeDatkindStr;
   node.dat.s.run = buf;
   node.dat.s.len = len;
   node.kind = kind;
   node.lnum = lnum;
   node.lpos = lpos;
   node.offset = 0;

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

static tree_t *graft_tree_i(tree_t *base, int i, irnodekind_t kind) {
   irnode_t node;

   node.datkind = IrnodeDatkindInt;
   node.dat.i = i;
   node.kind = kind;

   return tree_graft(base, graft_tree_common(&node, 0, 0));
}

static tree_t *graft_tree_ui(tree_t *base, unsigned int ui, irnodekind_t kind) {
   irnode_t node;

   node.datkind = IrnodeDatkindUint;
   node.dat.ui = ui;
   node.kind = kind;

   return tree_graft(base, graft_tree_common(&node, 0, 0));
}

static tree_t *graft_tree_opcode(
   tree_t *base,
   iropcode_t opcode,
   size_t lnum,
   size_t lpos
) {
   irnode_t node;

   node.datkind = IrnodeDatkindUint;
   node.dat.ui = opcode;
   node.kind = IrnodekindOpcode;

   return tree_graft(base,
      graft_tree_common(&node, lnum, lpos));
}

static tree_t *graft_tree_var(tree_t *base, irvar_t var) {
   return graft_tree_ui(base, var, IrnodekindVar);
}

static tree_t *graft_tree_charidx(tree_t *base, spl_uint_t charidx) {
   return graft_tree_ui(base, charidx, IrnodekindPerson);
}

static inline tree_t *graft_tree_common(
   irnode_t *node,
   size_t lnum,
   size_t lpos
) {
   node->lnum = lnum;
   node->lpos = lpos;
   node->offset = 0;

   return tree_plant(node, sizeof *node);
}

static void add_block(irgen_ctx_t *ictx, size_t cnt) {
   ictx->curr_block = graft_tree_ui(
      ictx->curr_scene,
      cnt,
      IrnodekindBlock
   );
}

extern size_t count_opcodes(tree_t *irt) {
   size_t counter;

   counter = 0;
   tree_pre_traverse(irt, opcode_counter, 0, &counter);

   return counter;
}

static void opcode_counter(tree_t *t, int lv, void *ctx) {
   irnode_t *n;
   size_t *counter;

   (void) lv;

   n = tree_dat(t);

   if (n->kind != IrnodekindBlock) {
      return;
   }

   counter = ctx;
   *counter += tree_clen(t);
}
