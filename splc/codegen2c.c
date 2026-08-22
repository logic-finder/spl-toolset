#include "codegen2c.h"
#include "codegen2c.internals.h"

// fixme: use resolve_var

extern void transpile2c(compile_ctx_t *cctx) {
   cg2c_ctx_t gctx;
   char *destname;

   gctx.irt = cctx->irt;
   gctx.dp = tree_child(gctx.irt, 0);
   gctx.nrtv = tree_child(gctx.irt, 1);
   gctx.pt = cctx->pt;
   gctx.title = tree_child(gctx.pt, 0);
   gctx.pt_dp = tree_child(gctx.pt, 1);

   destname = make_destname(cctx->ov->src, C_EXTENSION);
   gctx.fp = safe_fopen(destname, "w");

   if (cctx->of->vbs) safe_vprintf(
      ENPREFIX "transpiling into the target language " Cbwhite "C" Creset "..."
   );

   safe_fputs(gctx.fp, "#include <splrt/splrt.h>\n");
   codegen_title(&gctx);
   safe_fputs(gctx.fp, "int main(void) {\n");
   codegen_locals(&gctx);
   tree_pre_traverse(gctx.nrtv, codegen_route, 0, &gctx);
   safe_vfprintf(gctx.fp, "\n%scleanup_runtime(rctx);\n", indent);
   safe_vfprintf(gctx.fp, "%sreturn 0;\n", indent);
   safe_fputs(gctx.fp, "}\n");

   if (cctx->of->vbs) safe_vprintf(
      " " Cgreen "done!" Creset "\t(output=" Cbyellow "%s" Creset ")\n",
      destname
   );

   safe_fclose(gctx.fp);

   if (cctx->of->drn)
      safe_remove(destname);

   free(destname);
}

static void codegen_title(cg2c_ctx_t *gctx) {
   node_t *n;

   n = tree_dat(gctx->title);
   safe_vfprintf(gctx->fp, "\n/* %s */\n\n", n->dat.s.run);
}

static void codegen_locals(cg2c_ctx_t *gctx) {
   tree_t *opcode;
   irnode_t *irnode;
   size_t dpsz;
   node_t *node;

   opcode = tree_child(gctx->dp, 0);
   irnode = tree_chdat(opcode, 1);
   dpsz = tree_clen(gctx->pt_dp);

   safe_vfprintf(gctx->fp, "%srt_ctx_t *rctx = init_runtime(%u);\n",
      indent, irnode->dat.ui);

   for (size_t i = 0; i < dpsz; i++) {
      node = tree_chdat(gctx->pt_dp, i);
      safe_vfprintf(gctx->fp, "%sstage_setname(rctx->st, %u, \"%s\");\n",
         indent, i, node->dat.s.run);
   }
}

static void codegen_route(tree_t *t, int lv, void *ctx) {
   cg2c_ctx_t *gctx;

   (void) lv;

   gctx = ctx;
   gctx->t = t;
   gctx->n = tree_dat(t);

   switch (gctx->n->kind) {
      case IrnodekindScene  : handle_scene (gctx); break;
      case IrnodekindBlock  : handle_block (gctx); break;
      case IrnodekindOpcode : handle_opcode(gctx); break;
      default: ;
   }
}

static void handle_scene(cg2c_ctx_t *gctx) {
   tree_t *act;
   irnode_t *act_dat;

   act = tree_parent(gctx->t);
   act_dat = tree_dat(act);

   // fixme: use KEYWRD_ACT _SCENE
   safe_vfprintf(gctx->fp, "\nAct_%s_Scene_%s:\n",
      act_dat->dat.s.run, gctx->n->dat.s.run);
}

static void handle_block(cg2c_ctx_t *gctx) {
   tree_t *act, *scene;
   irnode_t *act_dat, *scene_dat;

   scene = tree_parent(gctx->t);
   act = tree_parent(scene);

   act_dat = tree_dat(act);
   scene_dat = tree_dat(scene);

   if (gctx->n->dat.ui == 0)
      return;

   safe_vfprintf(gctx->fp, "\nAct_%s_Scene_%s_L%u:\n",
      act_dat->dat.s.run,
      scene_dat->dat.s.run,
      gctx->n->dat.ui);
}

static void handle_opcode(cg2c_ctx_t *gctx) {
   switch (gctx->n->dat.ui) {
      case IropcodeSet    : codegen_set (gctx); break;
      case IropcodeAsgn   : codegen_asgn(gctx); break;
      case IropcodeEnter  : codegen_enterlike(gctx, "enter" ); break;
      case IropcodeExit   : codegen_enterlike(gctx, "exit"  ); break;
      case IropcodeExeunt : codegen_exeunt(gctx); break;
      case IropcodeSpeak  : codegen_speak(gctx); break;
      case IropcodePush   : codegen_push (gctx); break;
      case IropcodePop    : codegen_pop(gctx); break;
      case IropcodeSum    : codegen_binary_op(gctx, "sum" ); break;
      case IropcodeDiff   : codegen_binary_op(gctx, "diff"); break;
      case IropcodeProd   : codegen_binary_op(gctx, "prod"); break;
      case IropcodeQuot   : codegen_binary_op(gctx, "quot"); break;
      case IropcodeRem    : codegen_binary_op(gctx, "rem" ); break;
      case IropcodeSqrt   : codegen_unary_op(gctx, "sqrt"); break;
      case IropcodeSqur   : codegen_unary_op(gctx, "squr"); break;
      case IropcodeCube   : codegen_unary_op(gctx, "cube"); break;
      case Iropcode2x     : codegen_twice(gctx); break;
      case IropcodeFact   : codegen_unary_op(gctx, "fact"); break;
      case IropcodeOutN   : codegen_io(gctx, "outn"); break;
      case IropcodeOutC   : codegen_io(gctx, "outc"); break;
      case IropcodeInN    : codegen_io(gctx, "inn" ); break;
      case IropcodeInC    : codegen_io(gctx, "inc" ); break;
      case IropcodeGoto   : codegen_goto(gctx); break;
      case IropcodeEq     : codegen_comp(gctx, "=="); break;
      case IropcodeGt     : codegen_comp(gctx, ">" ); break;
      case IropcodeLt     : codegen_comp(gctx, "<" ); break;
      case IropcodeRememb : codegen_rememb(gctx); break;
      case IropcodeRecall : codegen_recall(gctx); break;
      case IropcodeJumpT  : codegen_jump(gctx, true ); break;
      case IropcodeJumpF  : codegen_jump(gctx, false); break;
      case IropcodeNegate : codegen_negate(gctx); break;
      default: ;  /* control never reaches default */
   }
}

static void codegen_set(cg2c_ctx_t *gctx) {
   irnode_t *p2;

   /* "SET const X" is possible only */
   p2 = tree_chdat(gctx->t, 1);

   safe_vfprintf(gctx->fp, "%srctx->cnst = %d;\n",
      indent, p2->dat.i);
}

static void codegen_asgn(cg2c_ctx_t *gctx) {
   irnode_t *p2;

   /* "ASGN const X" is possible only */
   p2 = tree_chdat(gctx->t, 1);

   switch (p2->kind) {
      case IrnodekindVar:
         if (p2->dat.ui == IrvarHearer) {
            safe_vfprintf(gctx->fp, "%sset_hearer(rctx);\n", indent);
            safe_vfprintf(gctx->fp,
               "%srctx->cnst = rctx->dp[rctx->h];\n",
               indent);
         }
         else {  /* IrvarTeller */
            safe_vfprintf(gctx->fp,
               "%srctx->cnst = rctx->dp[rctx->t];\n",
               indent);
         }
      break;

      case IrnodekindPerson:
         safe_vfprintf(gctx->fp, "%srctx->cnst = rctx->dp[%u];\n",
            indent, p2->dat.ui - Irvar_Dp_Begin);
      break;

      default: ;  /* control never reaches default */
   }
}

static void codegen_enterlike(cg2c_ctx_t *gctx, const char *op) {
   irnode_t *p1;

   p1 = tree_chdat(gctx->t, 0);

   safe_vfprintf(gctx->fp, "%sstage_%s(rctx->st, %u);\n",
      indent, op, p1->dat.ui);
}

static void codegen_exeunt(cg2c_ctx_t *gctx) {
   safe_vfprintf(gctx->fp, "%sstage_exeunt(rctx->st);\n", indent);
}

static void codegen_speak(cg2c_ctx_t *gctx) {
   irnode_t *p1;

   p1 = tree_chdat(gctx->t, 0);

   safe_vfprintf(gctx->fp,
      "%sassert_offstage(rctx, %u);\n",
      indent, p1->dat.ui);

   safe_vfprintf(gctx->fp, "%srctx->t = %d;\n", indent, p1->dat.ui);
}

static void codegen_push(cg2c_ctx_t *gctx) {
   irnode_t *p1;

   p1 = tree_chdat(gctx->t, 0);

   /* "PUSH teller|hearer|const" is only possible */
   safe_vfprintf(gctx->fp,
      "%sstack_push(rctx->s, %s);\n",
      indent,
      resolve_var(p1->dat.ui));
}

static void codegen_pop(cg2c_ctx_t *gctx) {
   irnode_t *p1;

   p1 = tree_chdat(gctx->t, 0);

   switch (p1->dat.ui) {
      case IrvarHearer:
         safe_vfprintf(gctx->fp, "%sset_hearer(rctx);\n", indent);
         safe_vfprintf(gctx->fp,
            "%sstack_pop(rctx->s, &rctx->dp[rctx->h]);\n", indent);
      break;

      case IrvarOperandL:
         safe_vfprintf(gctx->fp,
            "%sstack_pop(rctx->s, &rctx->ol);\n", indent);
      break;

      case IrvarOperandR:
         safe_vfprintf(gctx->fp,
            "%sstack_pop(rctx->s, &rctx->or);\n", indent);
      break;

      default: ;  /* control never reaches default */
   }
}

static void codegen_binary_op(cg2c_ctx_t *gctx, const char *op) {
   /* "<op> const l r" is only possible */
   safe_vfprintf(gctx->fp,
      "%srctx->cnst = op_%s(rctx->ol, rctx->or);\n", indent, op);
}

static void codegen_unary_op(cg2c_ctx_t *gctx, const char *op) {
   safe_vfprintf(gctx->fp,
      "%srctx->cnst = op_%s(rctx->ol);\n", indent, op);
}

static void codegen_twice(cg2c_ctx_t *gctx) {
   irnode_t *p2;

   p2 = tree_chdat(gctx->t, 1);

   safe_vfprintf(gctx->fp,
      "%srctx->cnst = op_2x(%s);\n",
      indent, resolve_var(p2->dat.ui));
}

static void codegen_io(cg2c_ctx_t *gctx, const char *op) {
   safe_vfprintf(gctx->fp, "%sio_%s(rctx);\n", indent, op);
}

static void codegen_goto(cg2c_ctx_t *gctx) {
   tree_t *act, *scene, *block;
   irnode_t *p1, *p2, *act_dat;

   p1 = tree_chdat(gctx->t, 0);
   p2 = tree_chdat(gctx->t, 1);

   switch (p1->dat.ui) {
      case NODEKIND_ACT:
         // fixme: use KEYWRD_ACT _SCENE
         safe_vfprintf(gctx->fp, "%sgoto Act_%s_Scene_I;\n",
            indent, p2->dat.s.run);
      break;

      case NODEKIND_SCENE:
         block = tree_parent(gctx->t);
         scene = tree_parent(block);
         act = tree_parent(scene);
         act_dat = tree_dat(act);
         // fixme: use KEYWRD_ACT _SCENE
         safe_vfprintf(gctx->fp, "%sgoto Act_%s_Scene_%s;\n",
            indent, act_dat->dat.s.run, p2->dat.s.run);
      break;

      default: ;  /* control never reaches default */
   }
}

static void codegen_comp(cg2c_ctx_t *gctx, const char *op) {
   safe_vfprintf(gctx->fp,
      "%srctx->cond = rctx->ol %s rctx->or;\n",
      indent, op);
}

static void codegen_rememb(cg2c_ctx_t *gctx) {
   irnode_t *n;

   n = tree_chdat(gctx->t, 0);

   /* Note: REMEMB const is only possible */
   safe_vfprintf(gctx->fp,
      "%srememb(rctx, %s);\n",
      indent, resolve_var(n->dat.ui));
}

static void codegen_recall(cg2c_ctx_t *gctx) {
   safe_vfprintf(gctx->fp, "%srecall(rctx);\n", indent);
}

static void codegen_jump(cg2c_ctx_t *gctx, bool v) {
   tree_t *act, *scene, *block;
   irnode_t *act_dat, *scene_dat, *op_dat;
   const char *neg;

   block = tree_parent(gctx->t);
   scene = tree_parent(block);
   act = tree_parent(scene);

   neg = v ? "" : "!";
   act_dat = tree_dat(act);
   scene_dat = tree_dat(scene);
   op_dat = tree_chdat(gctx->t, 0);

   safe_vfprintf(gctx->fp, "%sif (%srctx->cond) goto Act_%s_Scene_%s_L%u;\n",
      indent,
      neg,
      act_dat->dat.s.run,
      scene_dat->dat.s.run,
      op_dat->dat.ui
   );
}

static void codegen_negate(cg2c_ctx_t *gctx) {
   safe_vfprintf(gctx->fp, "%srctx->cond = !rctx->cond;\n", indent);
}

static const char *resolve_var(irvar_t var) {
   switch (var) {
      case IrvarTeller   : return "rctx->dp[rctx->t]";
      case IrvarHearer   : return "rctx->dp[rctx->h]";
      case IrvarConst    : return "rctx->cnst";
      case IrvarOperandL : return "rctx->ol";
      default: return NULL;
   }
}
