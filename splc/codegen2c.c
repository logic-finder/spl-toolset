#include "codegen2c.h"
#include "codegen2c.internals.h"

extern void transpile2c(void) {
   char *destname;
   FILE *fp;
   tree_t *dp, *nrtv;

   destname = make_destname("hello.spl", C_EXTENSION);
   fp = safe_fopen(destname, "w");

   fmtwrt(ENPREFIX "transpiling into the target language " Cbwhite "C" Creset "...");

   dp = tree_child(irt, 0);
   nrtv = tree_child(irt, 1);

   safe_fputs(fp, "#include <splrt/splrt.h>\n");
   codegen_title(fp);
   safe_fputs(fp, "int main(void) {\n");
   codegen_locals(dp, fp, tree_child(pt, 1));
   tree_pre_traverse(nrtv, codegen_route, 0, fp);
   ffmtwrt(fp, "%scleanup_runtime(rctx);\n", indent);
   ffmtwrt(fp, "\n%sreturn 0;\n", indent);
   safe_fputs(fp, "}\n");

   fmtwrt(" " Cgreen "done!" Creset "\n");

   safe_fclose(fp);
   free(destname);
}

static void codegen_title(FILE *fp) {
   tree_t *title;
   node_t *n;

   title = tree_child(pt, 0);
   n = tree_dat(title);

   ffmtwrt(fp, "\n/* %s */\n\n", n->dat.s.run);
}

static void codegen_locals(tree_t *irt_dp, FILE *fp, tree_t *pt_dp) {
   tree_t *opcode;
   irnode_t *irnode;
   size_t dpsz;
   node_t *node;

   opcode = tree_child(irt_dp, 0);
   irnode = tree_chdat(opcode, 1);
   dpsz = tree_clen(pt_dp);

   ffmtwrt(fp, "%srt_ctx_t *rctx = init_runtime(%u);\n",
      indent, irnode->dat.ui);

   for (size_t i = 0; i < dpsz; i++) {
      node = tree_chdat(pt_dp, i);
      ffmtwrt(fp, "%sstage_setname(rctx->st, %u, \"%s\");\n",
         indent, i, node->dat.s.run);
   }
}

static void codegen_route(tree_t *t, int lv, void *ctx) {
   irnode_t *n;
   FILE *fp;

   (void) lv;
   n = tree_dat(t);
   fp = ctx;

   switch (n->kind) {
      case IrnodekindScene  : handle_scene (t, fp); break;
      case IrnodekindBlock  : handle_block (t, fp); break;
      case IrnodekindOpcode : handle_opcode(t, fp); break;
      default: ;
   }
}

static void handle_scene(tree_t *t, FILE *fp) {
   tree_t *act;
   irnode_t *act_dat, *scene_dat;

   act = tree_parent(t);
   act_dat = tree_dat(act);
   scene_dat = tree_dat(t);

   // fixme: use KEYWRD_ACT _SCENE
   ffmtwrt(fp, "\nAct_%s_Scene_%s:\n",
      act_dat->dat.s.run, scene_dat->dat.s.run);
}

static void handle_block(tree_t *t, FILE *fp) {
   irnode_t *n = tree_dat(t);

   if (n->dat.ui == 0)
      return;

   ffmtwrt(fp, "\nL%u:\n", n->dat.ui);
}

static void handle_opcode(tree_t *t, FILE *fp) {
   irnode_t *n;

   n = tree_dat(t);

   switch (n->dat.ui) {
      case IropcodeSet    : codegen_set (t, fp); break;
      case IropcodeAsgn   : codegen_asgn(t, fp); break;
      case IropcodeEnter  : codegen_enterlike(t, fp, "enter" ); break;
      case IropcodeExit   : codegen_enterlike(t, fp, "exit"  ); break;
      case IropcodeExeunt : codegen_exeunt(fp); break;
      case IropcodeSpeak  : codegen_speak(t, fp); break;
      case IropcodePush   : codegen_push  (fp); break;
      case IropcodePop    : codegen_pop(t, fp); break;
      case IropcodeSum    : codegen_binary_op(fp, "sum" ); break;
      case IropcodeDiff   : codegen_binary_op(fp, "diff"); break;
      case IropcodeProd   : codegen_binary_op(fp, "prod"); break;
      case IropcodeQuot   : codegen_binary_op(fp, "quot"); break;
      case IropcodeRem    : codegen_binary_op(fp, "rem" ); break;
      case IropcodeSqrt   : codegen_unary_op(fp, "sqrt"); break;
      case IropcodeSqur   : codegen_unary_op(fp, "squr"); break;
      case IropcodeCube   : codegen_unary_op(fp, "cube"); break;
      case Iropcode2x     : codegen_unary_op(fp, "2x"  ); break;
      case IropcodeFact   : codegen_unary_op(fp, "fact"); break;
      case IropcodeOutN   : codegen_io(fp, "outn"); break;
      case IropcodeOutC   : codegen_io(fp, "outc"); break;
      case IropcodeInN    : codegen_io(fp, "inn" ); break;
      case IropcodeInC    : codegen_io(fp, "inc" ); break;
      case IropcodeGoto   : codegen_goto(t, fp); break;
      case IropcodeEq     : codegen_comp(fp, "=="); break;
      case IropcodeGt     : codegen_comp(fp, ">" ); break;
      case IropcodeLt     : codegen_comp(fp, "<" ); break;
      case IropcodeRememb : codegen_rememb(t, fp); break;
      case IropcodeRecall : codegen_recall(fp); break;
      case IropcodeJumpT  : codegen_jump(t, fp, true ); break;
      case IropcodeJumpF  : codegen_jump(t, fp, false); break;
      case IropcodeNegate : codegen_negate(fp); break;
      default: ;
   }
}

static void codegen_set(tree_t *t, FILE *fp) {
   irnode_t *p2;

   /* "SET const X" is possible only */
   p2 = tree_chdat(t, 1);

   ffmtwrt(fp, "%srctx->cnst = %d;\n",
      indent, p2->dat.i);
}

static void codegen_asgn(tree_t *t, FILE *fp) {
   irnode_t *p2;

   /* "ASGN const X" is possible only */
   p2 = tree_chdat(t, 1);

   switch (p2->kind) {
      case IrnodekindVar:
         if (p2->dat.ui == IrvarHearer) {
            ffmtwrt(fp, "%sset_hearer(rctx);\n", indent);
            ffmtwrt(fp,
               "%srctx->cnst = rctx->dp[rctx->h];\n",
               indent);
         }
         else {  /* IrvarTeller */
            ffmtwrt(fp,
               "%srctx->cnst = rctx->dp[rctx->t];\n",
               indent);
         }
      break;

      case IrnodekindPerson:
         ffmtwrt(fp, "%srctx->cnst = rctx->dp[%u];\n",
            indent, p2->dat.ui - Irvar_Dp_Begin);
      break;

      default: ;
   }
}

static void codegen_enterlike(tree_t *t, FILE *fp, const char *op) {
   irnode_t *p1;

   p1 = tree_chdat(t, 0);

   ffmtwrt(fp, "%sstage_%s(rctx->st, %u);\n",
      indent, op, p1->dat.ui);
}

static void codegen_exeunt(FILE *fp) {
   ffmtwrt(fp, "%sstage_exeunt(rctx->st);\n", indent);
}

static void codegen_speak(tree_t *t, FILE *fp) {
   irnode_t *p1 = tree_chdat(t, 0);
   ffmtwrt(fp,
      "%sassert_offstage(rctx, %u);\n",
      indent, p1->dat.ui);
   ffmtwrt(fp, "%srctx->t = %d;\n", indent, p1->dat.ui);
}

static void codegen_push(FILE *fp) {
   /* "PUSH const" is only possible */
   ffmtwrt(fp, "%sstack_push(rctx->s, rctx->cnst);\n", indent);
}

static void codegen_pop(tree_t *t, FILE *fp) {
   irnode_t *p1;

   p1 = tree_chdat(t, 0);

   switch (p1->dat.ui) {
      case IrvarHearer:
         // shutup: /* Note: seems redundant; suffice to set at codegen_asgn */
         ffmtwrt(fp, "%sset_hearer(rctx);\n", indent);
         ffmtwrt(fp,
            "%sstack_pop(rctx->s, &rctx->dp[rctx->h]);\n", indent);
      break;

      case IrvarOperandL:
         ffmtwrt(fp,
            "%sstack_pop(rctx->s, &rctx->ol);\n", indent);
      break;

      case IrvarOperandR:
         ffmtwrt(fp,
            "%sstack_pop(rctx->s, &rctx->or);\n", indent);
      break;

      default: ;
   }
}

static void codegen_binary_op(FILE *fp, const char *op) {
   /* "<op> const l r" is only possible */
   ffmtwrt(fp,
      "%srctx->cnst = op_%s(rctx->ol, rctx->or);\n", indent, op);
}

static void codegen_unary_op(FILE *fp, const char *op) {
   ffmtwrt(fp,
      "%srctx->cnst = op_%s(rctx->ol);\n", indent, op);
}

static void codegen_io(FILE *fp, const char *op) {
   ffmtwrt(fp, "%sio_%s(rctx);\n", indent, op);
}

static void codegen_goto(tree_t *t, FILE *fp) {
   tree_t *act, *scene, *block;
   irnode_t *p1, *p2, *act_dat;

   p1 = tree_chdat(t, 0);
   p2 = tree_chdat(t, 1);

   switch (p1->dat.ui) {
      case NODEKIND_ACT:
         // fixme: use KEYWRD_ACT _SCENE
         ffmtwrt(fp, "%sgoto Act_%s_Scene_I;\n",
            indent, p2->dat.s.run);
      break;

      case NODEKIND_SCENE:
         block = tree_parent(t);
         scene = tree_parent(block);
         act = tree_parent(scene);
         act_dat = tree_dat(act);
         // fixme: use KEYWRD_ACT _SCENE
         ffmtwrt(fp, "%sgoto Act_%s_Scene_%s;\n",
            indent, act_dat->dat.s.run, p2->dat.s.run);
      break;

      default: ;
   }
}

static void codegen_comp(FILE *fp, const char *op) {
   ffmtwrt(fp,
      "%srctx->cond = rctx->ol %s rctx->or;\n",
      indent, op);
}

static void codegen_rememb(tree_t *t, FILE *fp) {
   irnode_t *n = tree_dat(t);
   ffmtwrt(fp, "%srememb(rctx, %" SPL_INT_FMTSPC ");\n",
      indent, n->dat.i);
}

static void codegen_recall(FILE *fp) {
   ffmtwrt(fp, "%srecall(rctx);\n", indent);
}

static void codegen_jump(tree_t *t, FILE *fp, bool v) {
   const char *neg;
   irnode_t *n;

   neg = v ? "" : "!";
   n = tree_chdat(t, 0);

   ffmtwrt(fp, "%sif (%srctx->cond) goto L%u;\n",
      indent, neg, n->dat.ui);
}

static void codegen_negate(FILE *fp) {
   ffmtwrt(fp, "%srctx->cond = !rctx->cond;\n", indent);
}
