#include "codegen2c.h"
#include "codegen2c.internals.h"

// fixme: use resolve_var

extern bool transpile2c(compile_ctx_t *cctx) {
   bool transpile_stat = false;
   char *destname = NULL;

   cg2c_ctx_t gctx = {
      .irt = cctx->irt,
      .dp = tree_child(gctx.irt, 0),
      .nrtv = tree_child(gctx.irt, 1),
      .pt = cctx->pt,
      .title = tree_child(gctx.pt, 0),
      .pt_dp = tree_child(gctx.pt, 1),
      .ferr = NULL
   };

   int nchar;

   // Create a destination file
   if (!(destname = make_destname(cctx->sourcefile, C_EXTENSION))) {
      goto cleanup;
   }
   if (destname == cctx->sourcefile) {
      gctx.ferr = "empty filename";
      goto cleanup;
   }
   if (!(gctx.fp = fopen(destname, "w"))) {
      goto cleanup;
   }

   // Notify the start of the work
   if (cctx->cfg->vbs) {
      (void) printf(
         ENPREFIX "transpiling into the target language " Cbwhite "C" Creset "..."
      );
   }

   // Write C codes into the desination
   nchar = fprintf(gctx.fp, "#include <splrt/splrt.h>\n");
   if (nchar < 0) {
      goto fclose;
   }

   if (!codegen_title(&gctx)) {
      goto fclose;
   }

   nchar = fprintf(gctx.fp, "int main(void) {\n");
   if (nchar < 0) {
      goto fclose;
   }

   if (!codegen_locals(&gctx)) {
      goto fclose;
   }

   if (!tree_pre_traverse(gctx.nrtv, codegen_route, 0, &gctx)) {
      goto fclose;
   }

   nchar = fprintf(
      gctx.fp,
      "\n%scleanup_runtime(rctx);\n",
      indent
   );
   if (nchar < 0) {
      goto fclose;
   }

   nchar = fprintf(
      gctx.fp,
      "%sreturn 0;\n",
      indent
   );
   if (nchar < 0) {
      goto fclose;
   }

   nchar = fprintf(gctx.fp, "}\n");
   if (nchar < 0) {
      goto fclose;
   }

   // Notify the end of the work
   if (cctx->cfg->vbs) {
      (void) printf(
         " " Cgreen "done!" Creset "\t(output=" Cbyellow "%s" Creset ")\n",
         destname
      );
   }

   // Everything was successful
   transpile_stat = true;

fclose:
   if (fclose(gctx.fp)) {
      gctx.ferr = "failed to fclose";
      goto cleanup;
   }
   if (cctx->cfg->drn) {
      if (remove(destname)) {
         gctx.ferr = "failed to remove";
         goto cleanup;
      }
   }
cleanup:
   free(destname);

   cctx->ferr = gctx.ferr;
   return transpile_stat;
}

static bool codegen_title(cg2c_ctx_t *gctx) {
   node_t *n;
   int nchar;

   n = tree_dat(gctx->title);
   nchar = fprintf(
      gctx->fp,
      "\n/* %s */\n\n",
      n->dat.s.run
   );

   return (0 <= nchar) ? true : false;
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
      return false;
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
         return false;
      }
   }

   return true;
}

static bool codegen_route(tree_t *t, size_t lv, void *ctx) {
   cg2c_ctx_t *gctx = ctx;

   (void) lv;
   gctx->t = t;
   gctx->n = tree_dat(t);

   switch (gctx->n->kind) {
      case IrnodekindScene  : return handle_scene (gctx);
      case IrnodekindBlock  : return handle_block (gctx);
      case IrnodekindOpcode : return handle_opcode(gctx);
      default: break;
   }

   return false;
}

static bool handle_scene(cg2c_ctx_t *gctx) {
   tree_t *act = tree_parent(gctx->t);
   irnode_t *act_dat = tree_dat(act);

   int nchar;

   // fixme: use KEYWRD_ACT _SCENE
   nchar = fprintf(
      gctx->fp,
      "\nAct_%s_Scene_%s:\n",
      act_dat->dat.s.run,
      gctx->n->dat.s.run
   );

   return (0 <= nchar) ? true : false;
}

static bool handle_block(cg2c_ctx_t *gctx) {
   tree_t *scene = tree_parent(gctx->t);
   tree_t *act = tree_parent(scene);
   irnode_t *scene_dat = tree_dat(scene);
   irnode_t *act_dat = tree_dat(act);

   int nchar;

   // Do not emit the label for the 0th block
   if (gctx->n->dat.ui == 0) {
      return false;
   }

   // Write the label for nth block
   nchar = fprintf(
      gctx->fp,
      "\nAct_%s_Scene_%s_L%u:\n",
      act_dat->dat.s.run,
      scene_dat->dat.s.run,
      gctx->n->dat.ui
   );

   return (0 <= nchar) ? true : false;
}

static bool handle_opcode(cg2c_ctx_t *gctx) {
   // Route to each dedicated opcode handler
   switch (gctx->n->dat.ui) {
      case IropcodeSet    : return codegen_set(gctx);
      case IropcodeAsgn   : return codegen_asgn(gctx);
      case IropcodeEnter  : return codegen_enterlike(gctx, "enter");
      case IropcodeExit   : return codegen_enterlike(gctx, "exit");
      case IropcodeExeunt : return codegen_exeunt(gctx);
      case IropcodeSpeak  : return codegen_speak(gctx);
      case IropcodePush   : return codegen_push (gctx);
      case IropcodePop    : return codegen_pop(gctx);
      case IropcodeSum    : return codegen_binary_op(gctx, "sum");
      case IropcodeDiff   : return codegen_binary_op(gctx, "diff");
      case IropcodeProd   : return codegen_binary_op(gctx, "prod");
      case IropcodeQuot   : return codegen_binary_op(gctx, "quot");
      case IropcodeRem    : return codegen_binary_op(gctx, "rem");
      case IropcodeSqrt   : return codegen_unary_op(gctx, "sqrt");
      case IropcodeSqur   : return codegen_unary_op(gctx, "squr");
      case IropcodeCube   : return codegen_unary_op(gctx, "cube");
      case IropcodeFact   : return codegen_unary_op(gctx, "fact");
      case Iropcode2x     : return codegen_twice(gctx);
      case IropcodeOutN   : return codegen_io(gctx, "outn");
      case IropcodeOutC   : return codegen_io(gctx, "outc");
      case IropcodeInN    : return codegen_io(gctx, "inn");
      case IropcodeInC    : return codegen_io(gctx, "inc");
      case IropcodeGoto   : return codegen_goto(gctx);
      case IropcodeEq     : return codegen_comp(gctx, "==");
      case IropcodeGt     : return codegen_comp(gctx, ">");
      case IropcodeLt     : return codegen_comp(gctx, "<");
      case IropcodeRememb : return codegen_rememb(gctx);
      case IropcodeRecall : return codegen_recall(gctx);
      case IropcodeJumpT  : return codegen_jump(gctx, true);
      case IropcodeJumpF  : return codegen_jump(gctx, false);
      case IropcodeNegate : return codegen_negate(gctx);
      default: {
         gctx->ferr = "unknown opcode";
         return false;
      }
   }
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

   return (0 <= nchar) ? true : false;
}

static bool codegen_asgn(cg2c_ctx_t *gctx) {
   // only possible: ASGN const X
   irnode_t *p2 = tree_chdat(gctx->t, 1);

   int nchar;

   switch (p2->kind) {
      case IrnodekindVar: {
         switch (p2->dat.ui) {
            case IrvarHearer: {
               nchar = fprintf(
                  gctx->fp,
                  "%sset_hearer(rctx);\n",
                  indent
               );
               if (nchar < 0) {
                  return false;
               }
               nchar = fprintf(
                  gctx->fp,
                  "%srctx->cnst = rctx->dp[rctx->h];\n",
                  indent
               );
               if (nchar < 0) {
                  return false;
               }
            }
            break;

            case IrvarTeller: {
               nchar = fprintf(
                  gctx->fp,
                  "%srctx->cnst = rctx->dp[rctx->t];\n",
                  indent
               );
               if (nchar < 0) {
                  return false;
               }
            }
            break;

            default: {
               gctx->ferr = "unknown variable";
               return false;
            }
         }
      }
      break;

      case IrnodekindPerson: {
         // dp[0] is equal to irvar[dp_begin]
         if (p2->dat.ui < Irvar_Dp_Begin) {
            gctx->ferr = "corrupted dp[n]";
            return false;
         }
         nchar = fprintf(
            gctx->fp,
            "%srctx->cnst = rctx->dp[%u];\n",
            indent,
            p2->dat.ui - Irvar_Dp_Begin
         );
         if (nchar < 0) {
            return false;
         }
      }
      break;

      default: {
         gctx->ferr = "unknown p2->dat.ui";
         return false;
      }
   }
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

   return (0 <= nchar) ? true : false;
}

static bool codegen_exeunt(cg2c_ctx_t *gctx) {
   int nchar = fprintf(
      gctx->fp,
      "%sstage_exeunt(rctx->st);\n",
      indent
   );

   return (0 <= nchar) ? true : false;
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
      return false;
   }

   nchar = fprintf(
      gctx->fp,
      "%srctx->t = %d;\n",
      indent,
      p1->dat.ui
   );
   if (nchar < 0) {
      return false;
   }

   return true;
}

static bool codegen_push(cg2c_ctx_t *gctx) {
   irnode_t *p1 = tree_chdat(gctx->t, 0);
   const char *var = resolve_var(gctx, p1->dat.ui);

   if (!var) {
      return false;
   }

   // only possible: PUSH teller|hearer|const
   int nchar = fprintf(
      gctx->fp,
      "%sif (!stack_push(rctx->s, %s)) goto cleanup;\n",
      indent,
      var
   );

   return (0 <= nchar) ? true : false;
}

static bool codegen_pop(cg2c_ctx_t *gctx) {
   irnode_t *p1 = tree_chdat(gctx->t, 0);

   int nchar;

   switch (p1->dat.ui) {
      case IrvarHearer: {
         nchar = fprintf(
            gctx->fp,
            "%sset_hearer(rctx);\n",
            indent
         );
         if (nchar < 0) {
            return false;
         }
         nchar = fprintf(
            gctx->fp,
            "%sif (!stack_pop(rctx->s, &rctx->dp[rctx->h])) "
            "goto cleanup;\n",
            indent
         );
         if (nchar < 0) {
            return false;
         }
      }
      break;

      case IrvarOperandL: {
         nchar = fprintf(
            gctx->fp,
            "%sif (!stack_pop(rctx->s, &rctx->ol)) "
            "goto cleanup;\n",
            indent
         );
         if (nchar < 0) {
            return false;
         }
      }
      break;

      case IrvarOperandR: {
         nchar = fprintf(
            gctx->fp,
            "%sif (!stack_pop(rctx->s, &rctx->or)) "
            "goto cleanup;\n",
            indent
         );
         if (nchar < 0) {
            return false;
         }
      }
      break;

      default: {
         gctx->ferr = "unknown p1->dat.ui";
         return false;
      }
   }
}

static bool codegen_binary_op(cg2c_ctx_t *gctx, const char *op) {
   // only possible: <op> const l r
   int nchar = fprintf(
      gctx->fp,
      "%srctx->cnst = op_%s(rctx->ol, rctx->or);\n",
      indent,
      op
   );

   return (0 <= nchar) ? true : false;
}

static bool codegen_unary_op(cg2c_ctx_t *gctx, const char *op) {
   int nchar = fprintf(
      gctx->fp,
      "%srctx->cnst = op_%s(rctx->ol);\n",
      indent,
      op
   );

   return (0 <= nchar) ? true : false;
}

static bool codegen_twice(cg2c_ctx_t *gctx) {
   irnode_t *p2 = tree_chdat(gctx->t, 1);
   const char *var = resolve_var(gctx, p2->dat.ui);

   if (!var) {
      return false;
   }

   int nchar = fprintf(
      gctx->fp,
      "%srctx->cnst = op_2x(%s);\n",
      indent,
      var
   );

   return (0 <= nchar) ? true : false;
}

static bool codegen_io(cg2c_ctx_t *gctx, const char *op) {
   int nchar = fprintf(
      gctx->fp,
      "%sio_%s(rctx);\n",
      indent,
      op
   );

   return (0 <= nchar) ? true : false;
}

static bool codegen_goto(cg2c_ctx_t *gctx) {
   tree_t *act, *scene, *block;
   irnode_t *p1, *p2, *act_dat;
   int nchar;

   p1 = tree_chdat(gctx->t, 0);
   p2 = tree_chdat(gctx->t, 1);

   switch (p1->dat.ui) {
      case NODEKIND_ACT: {
         // fixme: use KEYWRD_ACT _SCENE
         nchar = fprintf(
            gctx->fp,
            "%sgoto Act_%s_Scene_I;\n",
            indent,
            p2->dat.s.run
         );
         if (nchar < 0) {
            return false;
         }
      }
      break;

      case NODEKIND_SCENE: {
         block = tree_parent(gctx->t);
         scene = tree_parent(block);
         act = tree_parent(scene);
         act_dat = tree_dat(act);

         // fixme: use KEYWRD_ACT _SCENE
         nchar = fprintf(
            gctx->fp,
            "%sgoto Act_%s_Scene_%s;\n",
            indent,
            act_dat->dat.s.run,
            p2->dat.s.run
         );
         if (nchar < 0) {
            return false;
         }
      }
      break;

      default: {
         gctx->ferr = "unknown p1->dat.ui";
         return false;
      }
   }
}

static bool codegen_comp(cg2c_ctx_t *gctx, const char *op) {
   int nchar = fprintf(
      gctx->fp,
      "%srctx->cond = rctx->ol %s rctx->or;\n",
      indent,
      op
   );

   return (0 <= nchar) ? true : false;
}

static bool codegen_rememb(cg2c_ctx_t *gctx) {
   irnode_t *n = tree_chdat(gctx->t, 0);
   const char *var = resolve_var(gctx, n->dat.ui);

   if (!var) {
      return false;
   }

   // only possible: REMEMB const
   int nchar = fprintf(
      gctx->fp,
      "%srememb(rctx, %s);\n",
      indent,
      var
   );

   return (0 <= nchar) ? true : false;
}

static bool codegen_recall(cg2c_ctx_t *gctx) {
   int nchar = fprintf(
      gctx->fp,
      "%srecall(rctx);\n",
      indent
   );

   return (0 <= nchar) ? true : false;
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

   return (0 <= nchar) ? true : false;
}

static bool codegen_negate(cg2c_ctx_t *gctx) {
   int nchar = fprintf(
      gctx->fp,
      "%srctx->cond = !rctx->cond;\n",
      indent
   );

   return (0 <= nchar) ? true : false;
}

static const char *resolve_var(cg2c_ctx_t *gctx, irvar_t var) {
   switch (var) {
      case IrvarTeller   : return "rctx->dp[rctx->t]";
      case IrvarHearer   : return "rctx->dp[rctx->h]";
      case IrvarConst    : return "rctx->cnst";
      case IrvarOperandL : return "rctx->ol";
      default: {
         gctx->ferr = "unknown variable";
         return NULL;
      }
   }
}
