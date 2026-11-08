#include "codegen2c.h"
#include "codegen2c.internals.h"

// fixme: use resolve_var
// fixme: buffer에 작성하고 나중에 한꺼번에 dest에 fwrite하는 것이 좋겠음

extern bool transpile2c(compile_ctx_t *cctx) {
   bool fclosed = false;
   cg2c_ctx_t *gctx = NULL;

   // Init gctx
   if (!(gctx = malloc(sizeof *gctx))) {
      cctx->perr = "failed to create gctx";
      PTRACE(cctx);
      return false;
   };

   gctx->srcname = cctx->sourcefile;
   gctx->destname = NULL;
   gctx->irt = cctx->irt;
   gctx->dp = tree_child(gctx->irt, 0);
   gctx->nrtv = tree_child(gctx->irt, 1);
   gctx->pt = cctx->pt;
   gctx->title = tree_child(gctx->pt, 0);
   gctx->pt_dp = tree_child(gctx->pt, 1);
   gctx->ptrace = cctx->ptrace;
   gctx->perr = NULL;
   gctx->phdlr = NULL;
   gctx->pnctx = NULL;

   // Create a destination file
   if (!create_dest(gctx)) {
      goto err;
   }

   // Notify the start of the work
   if (cctx->cfg->vbs) {
      (void) printf(
         ENPREFIX "transpiling into the target language " Cbwhite "C" Creset "..."
      );
   }

   // Write C codes into the desination
   if (!write2dest(gctx)) {
      goto err;
   }

   // Notify the end of the work
   if (cctx->cfg->vbs) {
      (void) printf(
         " " Cgreen "done!" Creset "\t(output=" Cbyellow "%s" Creset ")\n",
         gctx->destname
      );
   }

   // Clean up
   if (fclose(gctx->fp)) {
      fclosed = true;
      gctx->perr = "failed to fclose";
      goto err;
   }

   if (cctx->cfg->drn) {
      if (remove(gctx->destname)) {
         gctx->perr = "failed to remove dest";
         goto err;
      }
   }

   free(gctx->destname);

   // Normal execution path
   return true;

err:
   PTRACE(gctx);
   PH_COPY(cctx, gctx);
   FCLOSE_ERR_PATH(fclosed, gctx->fp);
   return false;
}

static char *create_dest(cg2c_ctx_t *gctx) {
   char *destname = NULL;

   // Make destname: basename(srcname) + ".c"
   // i.e. "hello.spl" -> "hello.c"
   if (!(destname = make_destname(gctx->srcname, C_EXTENSION))) {
      gctx->perr = "failed to make_destname";
      goto err;
   }
   if (destname == gctx->srcname) {
      gctx->perr = "problematic srcname";
      goto err;
   }

   // Fopen with destname
   if (!(gctx->fp = fopen(destname, "w"))) {
      gctx->perr = "failed fopen dest";
      goto err;
   }

   return destname;

err:
   PTRACE(gctx);
   return NULL;
}

static bool write2dest(cg2c_ctx_t *gctx) {
   int nchar;

   // This string is used in all panic situations
   // which occurred while writing to dest unless
   // overwritten to dedicated error message
   gctx->perr = "error occurred while writing to dest";

   /*
    * Begin writing...
    */

   // Write header
   nchar = fprintf(gctx->fp, "#include <splrt/splrt.h>\n");
   if (nchar < 0) {
      goto err;
   }

   // Write the title of spl program
   if (!codegen_title(gctx)) {
      goto err;
   }

   // Write the beginning of main
   nchar = fprintf(gctx->fp, "int main(void) {\n");
   if (nchar < 0) {
      goto err;
   }

   // Write local variables
   if (!codegen_locals(gctx)) {
      goto err;
   }

   // Translate IR into C by iterating the IR tree
   if (!tree_pre_traverse(gctx->nrtv, codegen_route, 0, gctx)) {
      goto err;
   }

   // Write cleanup code
   nchar = fprintf(
      gctx->fp,
      "\n%scleanup_runtime(rctx);\n",
      indent
   );
   if (nchar < 0) {
      goto err;
   }

   // Write return 0 for main
   nchar = fprintf(
      gctx->fp,
      "%sreturn 0;\n",
      indent
   );
   if (nchar < 0) {
      goto err;
   }

   // Write the end of main
   nchar = fprintf(gctx->fp, "}\n");
   if (nchar < 0) {
      goto err;
   }

   // DONE!
   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_title(cg2c_ctx_t *gctx) {
   node_t *n = tree_dat(gctx->title);

   int nchar = fprintf(
      gctx->fp,
      "\n/* %s */\n\n",
      n->dat.s.run
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_locals(cg2c_ctx_t *gctx) {
   tree_t *opcode = tree_child(gctx->dp, 0);
   irnode_t *irnode = tree_chdat(opcode, 1);
   size_t dpsz = tree_clen(gctx->pt_dp);

   node_t *node;
   int nchar;

   nchar = fprintf(
      gctx->fp,
      "%srt_ctx_t *rctx = init_runtime(%u);\n",
      indent,
      irnode->dat.ui
   );
   if (nchar < 0) {
      goto err;
   }

   for (size_t i = 0; i < dpsz; i++) {
      node = tree_chdat(gctx->pt_dp, i);
      nchar = fprintf(
         gctx->fp,
         "%sstage_setname(rctx->st, %u, \"%s\");\n",
         indent,
         i,
         node->dat.s.run
      );
      if (nchar < 0) {
         goto err;
      }
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_route(tree_t *t, size_t lv, void *ctx) {
   cg2c_ctx_t *gctx = ctx;

   bool hdlr_ret;

   (void) lv;
   gctx->t = t;
   gctx->n = tree_dat(t);

   switch (gctx->n->kind) {
      case IrnodekindScene: {
         hdlr_ret = handle_scene(gctx);
         break;
      }
      case IrnodekindBlock: {
         hdlr_ret = handle_block(gctx);
         break;
      }
      case IrnodekindOpcode: {
         hdlr_ret = handle_opcode(gctx);
         break;
      }
      default: {
         gctx->perr = "unknown irnodekind";
         hdlr_ret = false;
         break;
      }
   }
   if (!hdlr_ret) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool handle_scene(cg2c_ctx_t *gctx) {
   tree_t *act = tree_parent(gctx->t);
   irnode_t *act_dat = tree_dat(act);

   // fixme: use KEYWRD_ACT _SCENE
   int nchar = fprintf(
      gctx->fp,
      "\nAct_%s_Scene_%s:\n",
      act_dat->dat.s.run,
      gctx->n->dat.s.run
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool handle_block(cg2c_ctx_t *gctx) {
   tree_t *scene = tree_parent(gctx->t);
   tree_t *act = tree_parent(scene);
   irnode_t *scene_dat = tree_dat(scene);
   irnode_t *act_dat = tree_dat(act);

   // Do not emit the label for the 0th block
   if (gctx->n->dat.ui == 0) {
      return false;
   }

   // Write the label for nth block
   int nchar = fprintf(
      gctx->fp,
      "\nAct_%s_Scene_%s_L%u:\n",
      act_dat->dat.s.run,
      scene_dat->dat.s.run,
      gctx->n->dat.ui
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool handle_opcode(cg2c_ctx_t *gctx) {
   bool hdlr_ret;

   switch (gctx->n->dat.ui) {
      case IropcodeSet: {
         hdlr_ret = codegen_set(gctx);
         break;
      }
      case IropcodeAsgn: {
         hdlr_ret = codegen_asgn(gctx);
         break;
      }
      case IropcodeEnter: {
         hdlr_ret = codegen_enterlike(gctx, "enter");
         break;
      }
      case IropcodeExit: {
         hdlr_ret = codegen_enterlike(gctx, "exit");
         break;
      }
      case IropcodeExeunt: {
         hdlr_ret = codegen_exeunt(gctx);
         break;
      }
      case IropcodeSpeak: {
         hdlr_ret = codegen_speak(gctx);
         break;
      }
      case IropcodePush: {
         hdlr_ret = codegen_push(gctx);
         break;
      }
      case IropcodePop: {
         hdlr_ret = codegen_pop(gctx);
         break;
      }
      case IropcodeSum: {
         hdlr_ret = codegen_binary_op(gctx, "sum");
         break;
      }
      case IropcodeDiff: {
         hdlr_ret = codegen_binary_op(gctx, "diff");
         break;
      }
      case IropcodeProd: {
         hdlr_ret = codegen_binary_op(gctx, "prod");
         break;
      }
      case IropcodeQuot: {
         hdlr_ret = codegen_binary_op(gctx, "quot");
         break;
      }
      case IropcodeRem: {
         hdlr_ret = codegen_binary_op(gctx, "rem");
         break;
      }
      case IropcodeSqrt: {
         hdlr_ret = codegen_unary_op(gctx, "sqrt");
         break;
      }
      case IropcodeSqur: {
         hdlr_ret = codegen_unary_op(gctx, "squr");
         break;
      }
      case IropcodeCube: {
         hdlr_ret = codegen_unary_op(gctx, "cube");
         break;
      }
      case IropcodeFact: {
         hdlr_ret = codegen_unary_op(gctx, "fact");
         break;
      }
      case Iropcode2x: {
         hdlr_ret = codegen_twice(gctx);
         break;
      }
      case IropcodeOutN: {
         hdlr_ret = codegen_io(gctx, "outn");
         break;
      }
      case IropcodeOutC: {
         hdlr_ret = codegen_io(gctx, "outc");
         break;
      }
      case IropcodeInN: {
         hdlr_ret = codegen_io(gctx, "inn");
         break;
      }
      case IropcodeInC: {
         hdlr_ret = codegen_io(gctx, "inc");
         break;
      }
      case IropcodeGoto: {
         hdlr_ret = codegen_goto(gctx);
         break;
      }
      case IropcodeEq: {
         hdlr_ret = codegen_comp(gctx, "==");
         break;
      }
      case IropcodeGt: {
         hdlr_ret = codegen_comp(gctx, ">");
         break;
      }
      case IropcodeLt: {
         hdlr_ret = codegen_comp(gctx, "<");
         break;
      }
      case IropcodeRememb: {
         hdlr_ret = codegen_rememb(gctx);
         break;
      }
      case IropcodeRecall: {
         hdlr_ret = codegen_recall(gctx);
         break;
      }
      case IropcodeJumpT: {
         hdlr_ret = codegen_jump(gctx, true);
         break;
      }
      case IropcodeJumpF: {
         hdlr_ret = codegen_jump(gctx, false);
         break;
      }
      case IropcodeNegate: {
         hdlr_ret = codegen_negate(gctx);
         break;
      }
      default: {
         gctx->perr = "unknown opcode";
         hdlr_ret = false;
         break;
      }
   }
   if (!hdlr_ret) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_set(cg2c_ctx_t *gctx) {
   // only possible: SET const X
   irnode_t *p2 = tree_chdat(gctx->t, 1);

   int nchar = fprintf(
      gctx->fp,
      "%srctx->cnst = %d;\n",
      indent,
      p2->dat.i
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_asgn(cg2c_ctx_t *gctx) {
   // only possible: ASGN const X
   irnode_t *p2 = tree_chdat(gctx->t, 1);

   bool hdlr_ret;

   switch (p2->kind) {
      case IrnodekindVar: {
         hdlr_ret = codegen_asgn_var(gctx, p2);
         break;
      }
      case IrnodekindPerson: {
         hdlr_ret = codegen_asgn_person(gctx, p2);
         break;
      }
      default: {
         gctx->perr = "unknown p2->dat.ui";
         hdlr_ret = false;
         break;
      }
   }
   if (!hdlr_ret) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_asgn_var(cg2c_ctx_t *gctx, irnode_t *p2) {
   bool hdlr_ret;

   switch (p2->dat.ui) {
      case IrvarHearer: {
         hdlr_ret = codegen_asgn_var_hearer(gctx);
         break;
      }
      case IrvarTeller: {
         hdlr_ret = codegen_asgn_var_teller(gctx);
         break;
      }
      default: {
         gctx->perr = "unknown variable";
         hdlr_ret = false;
         break;
      }
   }
   if (!hdlr_ret) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_asgn_var_hearer(cg2c_ctx_t *gctx) {
   int nchar;

   nchar = fprintf(
      gctx->fp,
      "%sset_hearer(rctx);\n",
      indent
   );
   if (nchar < 0) {
      goto err;
   }

   nchar = fprintf(
      gctx->fp,
      "%srctx->cnst = rctx->dp[rctx->h];\n",
      indent
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_asgn_var_teller(cg2c_ctx_t *gctx) {
   int nchar = fprintf(
      gctx->fp,
      "%srctx->cnst = rctx->dp[rctx->t];\n",
      indent
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_asgn_person(cg2c_ctx_t *gctx, irnode_t *p2) {
   int nchar;

   // dp[0] is equal to irvar[dp_begin]
   if (p2->dat.ui < Irvar_Dp_Begin) {
      gctx->perr = "corrupted dp[n]";
      goto err;
   }

   nchar = fprintf(
      gctx->fp,
      "%srctx->cnst = rctx->dp[%u];\n",
      indent,
      p2->dat.ui - Irvar_Dp_Begin
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_enterlike(cg2c_ctx_t *gctx, const char *op) {
   irnode_t *p1 = tree_chdat(gctx->t, 0);

   int nchar = fprintf(
      gctx->fp,
      "%sstage_%s(rctx->st, %u);\n",
      indent,
      op,
      p1->dat.ui
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_exeunt(cg2c_ctx_t *gctx) {
   int nchar = fprintf(
      gctx->fp,
      "%sstage_exeunt(rctx->st);\n",
      indent
   );
   if (nchar < 0) {
      goto err;
   }

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_speak(cg2c_ctx_t *gctx) {
   irnode_t *p1 = tree_chdat(gctx->t, 0);

   int nchar;

   nchar = fprintf(
      gctx->fp,
      "%sassert_offstage(rctx, %u);\n",
      indent,
      p1->dat.ui
   );
   if (nchar < 0) {
      goto err;
   }

   nchar = fprintf(
      gctx->fp,
      "%srctx->t = %d;\n",
      indent,
      p1->dat.ui
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_push(cg2c_ctx_t *gctx) {
   irnode_t *p1 = tree_chdat(gctx->t, 0);
   const char *var = resolve_var(gctx, p1->dat.ui);

   if (!var) {
      gctx->perr = "unknown var";
      goto err;
   }

   // only possible: PUSH teller|hearer|const
   int nchar = fprintf(
      gctx->fp,
      "%sif (!stack_push(rctx->s, %s)) goto cleanup;\n",
      indent,
      var
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_pop(cg2c_ctx_t *gctx) {
   irnode_t *p1 = tree_chdat(gctx->t, 0);

   bool hdlr_ret;

   switch (p1->dat.ui) {
      case IrvarHearer: {
         hdlr_ret = codegen_pop_hearer(gctx);
         break;
      }
      case IrvarOperandL: {
         hdlr_ret = codegen_pop_ol(gctx);
         break;
      }
      case IrvarOperandR: {
         hdlr_ret = codegen_pop_or(gctx);
         break;
      }
      default: {
         gctx->perr = "unknown p1->dat.ui";
         hdlr_ret = false;
         break;
      }
   }
   if (!hdlr_ret) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_pop_hearer(cg2c_ctx_t *gctx) {
   int nchar;

   nchar = fprintf(
      gctx->fp,
      "%sset_hearer(rctx);\n",
      indent
   );
   if (nchar < 0) {
      goto err;
   }

   nchar = fprintf(
      gctx->fp,
      "%sif (!stack_pop(rctx->s, &rctx->dp[rctx->h])) "
      "goto cleanup;\n",
      indent
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_pop_ol(cg2c_ctx_t *gctx) {
   int nchar = fprintf(
      gctx->fp,
      "%sif (!stack_pop(rctx->s, &rctx->ol)) "
      "goto cleanup;\n",
      indent
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_pop_or(cg2c_ctx_t *gctx) {
   int nchar = fprintf(
      gctx->fp,
      "%sif (!stack_pop(rctx->s, &rctx->or)) "
      "goto cleanup;\n",
      indent
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_binary_op(cg2c_ctx_t *gctx, const char *op) {
   // only possible: <op> const l r
   int nchar = fprintf(
      gctx->fp,
      "%srctx->cnst = op_%s(rctx->ol, rctx->or);\n",
      indent,
      op
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_unary_op(cg2c_ctx_t *gctx, const char *op) {
   int nchar = fprintf(
      gctx->fp,
      "%srctx->cnst = op_%s(rctx->ol);\n",
      indent,
      op
   );
   if (nchar < 0) {
      goto err;
   }

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_twice(cg2c_ctx_t *gctx) {
   irnode_t *p2 = tree_chdat(gctx->t, 1);
   const char *var = resolve_var(gctx, p2->dat.ui);

   if (!var) {
      gctx->perr = "unknown var";
      goto err;
   }

   int nchar = fprintf(
      gctx->fp,
      "%srctx->cnst = op_2x(%s);\n",
      indent,
      var
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_io(cg2c_ctx_t *gctx, const char *op) {
   int nchar = fprintf(
      gctx->fp,
      "%sio_%s(rctx);\n",
      indent,
      op
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_goto(cg2c_ctx_t *gctx) {
   irnode_t *p1 = tree_chdat(gctx->t, 0);
   irnode_t *p2 = tree_chdat(gctx->t, 1);

   bool hdlr_ret;

   switch (p1->dat.ui) {
      case NODEKIND_ACT: {
         hdlr_ret = codegen_goto_act(gctx, p2->dat.s.run);
         break;
      }
      case NODEKIND_SCENE: {
         hdlr_ret = codegen_goto_scene(gctx, p2->dat.s.run);
         break;
      }
      default: {
         gctx->perr = "unknown p1->dat.ui";
         hdlr_ret = false;
         break;
      }
   }
   if (!hdlr_ret) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_goto_act(cg2c_ctx_t *gctx, const char *romnum) {
   // fixme: use KEYWRD_ACT _SCENE
   int nchar = fprintf(
      gctx->fp,
      "%sgoto Act_%s_Scene_I;\n",
      indent,
      romnum
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_goto_scene(cg2c_ctx_t *gctx, const char *romnum) {
   tree_t *block = tree_parent(gctx->t);
   tree_t *scene = tree_parent(block);
   tree_t *act = tree_parent(scene);

   irnode_t *act_dat = tree_dat(act);

   // fixme: use KEYWRD_ACT _SCENE
   int nchar = fprintf(
      gctx->fp,
      "%sgoto Act_%s_Scene_%s;\n",
      indent,
      act_dat->dat.s.run,
      romnum
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_comp(cg2c_ctx_t *gctx, const char *op) {
   int nchar = fprintf(
      gctx->fp,
      "%srctx->cond = rctx->ol %s rctx->or;\n",
      indent,
      op
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_rememb(cg2c_ctx_t *gctx) {
   irnode_t *n = tree_chdat(gctx->t, 0);
   const char *var = resolve_var(gctx, n->dat.ui);

   if (!var) {
      gctx->perr = "unknown var";
      goto err;
   }

   // only possible: REMEMB const
   int nchar = fprintf(
      gctx->fp,
      "%srememb(rctx, %s);\n",
      indent,
      var
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_recall(cg2c_ctx_t *gctx) {
   int nchar = fprintf(
      gctx->fp,
      "%srecall(rctx);\n",
      indent
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_jump(cg2c_ctx_t *gctx, bool v) {
   tree_t *block = tree_parent(gctx->t);
   tree_t *scene = tree_parent(block);
   tree_t *act = tree_parent(scene);

   irnode_t *op_dat = tree_chdat(gctx->t, 0);
   irnode_t *scene_dat = tree_dat(scene);
   irnode_t *act_dat = tree_dat(act);

   const char *neg = v ? "" : "!";

   int nchar = fprintf(
      gctx->fp,
      "%sif (%srctx->cond) goto Act_%s_Scene_%s_L%u;\n",
      indent,
      neg,
      act_dat->dat.s.run,
      scene_dat->dat.s.run,
      op_dat->dat.ui
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static bool codegen_negate(cg2c_ctx_t *gctx) {
   int nchar = fprintf(
      gctx->fp,
      "%srctx->cond = !rctx->cond;\n",
      indent
   );
   if (nchar < 0) {
      goto err;
   }

   return true;

err:
   PTRACE(gctx);
   return false;
}

static const char *resolve_var(cg2c_ctx_t *gctx, irvar_t var) {
   const char *ret;

   switch (var) {
      case IrvarTeller: {
         ret = "rctx->dp[rctx->t]";
         break;
      }
      case IrvarHearer: {
         ret = "rctx->dp[rctx->h]";
         break;
      }
      case IrvarConst: {
         ret = "rctx->cnst";
         break;
      }
      case IrvarOperandL: {
         ret = "rctx->ol";
         break;
      }
      default: {
         gctx->perr = "unknown variable";
         ret = NULL;
         break;
      }
   }
   if (!ret) {
      goto err;
   }

   return ret;

err:
   PTRACE(gctx);
   return NULL;
}
