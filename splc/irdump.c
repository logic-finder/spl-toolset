#include "irdump.h"
#include "irdump.internals.h"

extern void irdump(compile_ctx_t *cctx) {
   irdump_ctx_t ictx;

   char *destname;

   ictx.of = cctx->of;
   ictx.irt = cctx->irt;
   ictx.nrtv = tree_child(ictx.irt, 1);

   destname = make_destname("hello.spl", IR_EXTENSION);
   ictx.fp = safe_fopen(destname, "w");

   safe_vprintf(ENPREFIX "dumping IR into " Cbyellow "\"%s\"" Creset "...", destname);
   tree_pre_traverse(ictx.nrtv, route, 0, &ictx);
   safe_vprintf(" " Cgreen "done!" Creset "\n");

   safe_fclose(ictx.fp);

   if (cctx->of->drn)
      safe_remove(destname);

   free(destname);
}

static void route(tree_t *t, int lv, void *ctx) {
   irdump_ctx_t *ictx;

   (void) lv;

   ictx = ctx;
   ictx->t = t;
   ictx->n = tree_dat(t);

   switch (ictx->n->kind) {
      case IrnodekindOpcode : handle_opcode(ictx); break;
      case IrnodekindAct    : handle_act   (ictx); break;
      case IrnodekindScene  : handle_scene (ictx); break;
      case IrnodekindBlock  : handle_block (ictx); break;
      default: ;
   }
}

static void handle_opcode(irdump_ctx_t *ictx) {
   switch (ictx->n->dat.ui) {
      case IropcodeSet    : /* fall-through */
      case IropcodeAsgn   : handle_setlike  (ictx); break;
      case IropcodeEnter  : /* fall-through */
      case IropcodeExit   : /* fall-through */
      case IropcodeSpeak  : handle_enterlike(ictx); break;
      case IropcodeOutN   : /* fall-through */
      case IropcodeOutC   : /* fall-through */
      case IropcodeInN    : /* fall-through */
      case IropcodeInC    : /* fall-through */
      case IropcodeRecall : /* fall-through */
      case IropcodeNegate : /* fall-through */
      case IropcodeExeunt : handle_paramless_opcode(ictx); break;
      case IropcodeRememb : /* fall-through */
      case IropcodePush   : /* fall-through */
      case IropcodePop    : handle_pushlike(ictx); break;
      case IropcodeSum    : /* fall-through */
      case IropcodeDiff   : /* fall-through */
      case IropcodeProd   : /* fall-through */
      case IropcodeQuot   : /* fall-through */
      case IropcodeRem    : handle_binary_op(ictx); break;
      case IropcodeSqrt   : /* fall-through */
      case IropcodeSqur   : /* fall-through */
      case IropcodeCube   : /* fall-through */
      case Iropcode2x     : /* fall-through */
      case IropcodeFact   : /* fall-through */
      case IropcodeEq     : /* fall-through */
      case IropcodeGt     : /* fall-through */
      case IropcodeLt     : handle_unary_op(ictx); break;
      case IropcodeGoto   : handle_goto(ictx); break;
      case IropcodeJumpT  : /* fall-through */
      case IropcodeJumpF  : handle_jumplike(ictx); break;
      default: ;  /* control never reaches default */
   }
}

static void handle_act(irdump_ctx_t *ictx) {
   ictx->curr_act = ictx->n->dat.s.run;
}

static void handle_scene(irdump_ctx_t *ictx) {
   safe_fputc(ictx->fp, '\n');
   if (ictx->of->dbg) emit_debug_data(ictx);
   safe_vfprintf(
      ictx->fp,
      "Act_%s_Scene_%s:\n", // fixme: use KEYWRD_ACT _SCENE
      ictx->curr_act,
      ictx->n->dat.s.run
   );
}

static void handle_block(irdump_ctx_t *ictx) {
   if (!ictx->n->dat.ui)
      return;

   safe_vfprintf(
      ictx->fp,
      "\n.L%d:\n",
      ictx->n->dat.ui
   );
}

static void handle_setlike(irdump_ctx_t *ictx) {
   irnode_t *p1, *p2;

   p1 = tree_chdat(ictx->t, 0);
   p2 = tree_chdat(ictx->t, 1);

   if (ictx->of->dbg) emit_debug_data(ictx);

   switch (p2->kind) {
      // fixme: 별도 함수로 빼고 return하기
      case IrnodekindPerson : goto person;
      case IrnodekindVar    : goto var;
      case IrnodekindConst  : goto cnst;
      default: return;  /* control never reaches default */
   }

person:
   safe_vfprintf(
      ictx->fp,
      "%s%s %s dp[%d]\n",
      INDENT,
      resolve_opcode(ictx->n->dat.ui),
      resolve_var(p1->dat.ui),
      p2->dat.ui - Irvar_Dp_Begin
   );
   return;

var:
   safe_vfprintf(
      ictx->fp,
      "%s%s %s %s\n",
      INDENT,
      resolve_opcode(ictx->n->dat.ui),
      resolve_var(p1->dat.ui),
      resolve_var(p2->dat.ui)
   );
   return;

cnst:
   safe_vfprintf(
      ictx->fp,
      "%s%s %s %d\n",
      INDENT,
      resolve_opcode(ictx->n->dat.ui),
      resolve_var(p1->dat.ui),
      p2->dat.i
   );
}

static void handle_enterlike(irdump_ctx_t *ictx) {
   irnode_t *p1;

   p1 = tree_chdat(ictx->t, 0);

   if (ictx->of->dbg) emit_debug_data(ictx);
   safe_vfprintf(
      ictx->fp,
      "%s%s %d\n",
      INDENT,
      resolve_opcode(ictx->n->dat.ui),
      p1->dat.ui
   );
}

static void handle_pushlike(irdump_ctx_t *ictx) {
   irnode_t *p1;

   p1 = tree_chdat(ictx->t, 0);

   if (ictx->of->dbg) emit_debug_data(ictx);
   safe_vfprintf(
      ictx->fp,
      "%s%s %s\n",
      INDENT,
      resolve_opcode(ictx->n->dat.ui),
      resolve_var(p1->dat.ui)
   );
}

static void handle_goto(irdump_ctx_t *ictx) {
   irnode_t *p1, *p2, *act_dat;
   tree_t *act;

   p1 = tree_chdat(ictx->t, 0);
   p2 = tree_chdat(ictx->t, 1);

   if (ictx->of->dbg) emit_debug_data(ictx);

   if (p1->dat.ui == NODEKIND_ACT) {
      safe_vfprintf(
         ictx->fp,
         "%s%s Act_%s_Scene_I\n",  // fixme: use KEYWRD_ACT _SCENE
         INDENT,
         resolve_opcode(ictx->n->dat.ui),
         p2->dat.s.run
      );
      return;
   }

   /* NODEKIND_SCENE */
   act = tree_parent(tree_parent(tree_parent(ictx->t)));
   act_dat = tree_dat(act);

   safe_vfprintf(
      ictx->fp,
      "%s%s Act_%s_Scene_%s\n",  // fixme: use KEYWRD_ACT _SCENE
      INDENT,
      resolve_opcode(ictx->n->dat.ui),
      act_dat->dat.s.run,
      p2->dat.s.run
   );
}

static void handle_jumplike(irdump_ctx_t *ictx) {
   irnode_t *p1;

   p1 = tree_chdat(ictx->t, 0);

   if (ictx->of->dbg) emit_debug_data(ictx);
   safe_vfprintf(
      ictx->fp,
      "%s%s .L%d\n",
      INDENT,
      resolve_opcode(ictx->n->dat.ui),
      p1->dat.ui
   );
}

static void handle_binary_op(irdump_ctx_t *ictx) {
   irnode_t *p1, *p2, *p3;

   p1 = tree_chdat(ictx->t, 0);
   p2 = tree_chdat(ictx->t, 1);
   p3 = tree_chdat(ictx->t, 2);

   if (ictx->of->dbg) emit_debug_data(ictx);
   safe_vfprintf(
      ictx->fp,
      "%s%s %s %s %s\n",
      INDENT,
      resolve_opcode(ictx->n->dat.ui),
      resolve_var(p1->dat.ui),
      resolve_var(p2->dat.ui),
      resolve_var(p3->dat.ui)
   );
}

static void handle_unary_op(irdump_ctx_t *ictx) {
   irnode_t *p1, *p2;

   p1 = tree_chdat(ictx->t, 0);
   p2 = tree_chdat(ictx->t, 1);

   if (ictx->of->dbg) emit_debug_data(ictx);
   safe_vfprintf(
      ictx->fp,
      "%s%s %s %s\n",
      INDENT,
      resolve_opcode(ictx->n->dat.ui),
      resolve_var(p1->dat.ui),
      resolve_var(p2->dat.ui)
   );
}

static void handle_paramless_opcode(irdump_ctx_t *ictx) {
   if (ictx->of->dbg) emit_debug_data(ictx);
   safe_vfprintf(
      ictx->fp,
      "%s%s\n",
      INDENT,
      resolve_opcode(ictx->n->dat.ui)
   );
}

// fixme: 명령어의 윗줄이 아니라 명령어의 맨 끝에 위치하도록 변경하자
static inline void emit_debug_data(irdump_ctx_t *ictx) {
   safe_vfprintf(ictx->fp, "; %d:%d\n", ictx->n->lnum, ictx->n->lpos);
}

static const char *resolve_opcode(iropcode_t opcode) {
   switch (opcode) {
      case IropcodeSet     : return "SET";
      case IropcodeAsgn    : return "ASGN";
      case IropcodeEnter   : return "ENTER";
      case IropcodeExit    : return "EXIT";
      case IropcodeExeunt  : return "EXEUNT";
      case IropcodeSpeak   : return "SPEAK";
      case IropcodePush    : return "PUSH";
      case IropcodePop     : return "POP";
      case IropcodeSum     : return "SUM";
      case IropcodeDiff    : return "DIFF";
      case IropcodeProd    : return "PROD";
      case IropcodeQuot    : return "QUOT";
      case IropcodeRem     : return "REM";
      case IropcodeSqrt    : return "SQRT";
      case IropcodeSqur    : return "SQUR";
      case IropcodeCube    : return "CUBE";
      case Iropcode2x      : return "2X";
      case IropcodeFact    : return "FACT";
      case IropcodeOutN    : return "OUT_N";
      case IropcodeOutC    : return "OUT_C";
      case IropcodeInN     : return "IN_N";
      case IropcodeInC     : return "IN_C";
      case IropcodeGoto    : return "GOTO";
      case IropcodeEq      : return "EQ";
      case IropcodeGt      : return "GT";
      case IropcodeLt      : return "LT";
      case IropcodeRememb  : return "REMEMB";
      case IropcodeRecall  : return "RECALL";
      case IropcodeJumpT   : return "JUMPTRUE";
      case IropcodeJumpF   : return "JUMPFALSE";
      case IropcodeNegate  : return "NEGATE";
      default: return NULL;
   }
}

static const char *resolve_var(irvar_t var) {
   switch (var) {
      case IrvarDpsz     : return "dpsz";
      case IrvarTeller   : return "teller";
      case IrvarHearer   : return "hearer";
      case IrvarConst    : return "const";
      case IrvarOperandL : return "operand_l";
      case IrvarOperandR : return "operand_r";
      default: return NULL;
   }
}

/*** DEBUG ***/

extern void showirt(compile_ctx_t *cctx) {
   tree_pre_traverse(cctx->irt, showirnode, 0, NULL);
}

static void showirnode(tree_t *t, int lv, void *ctx) {
   static char buf[128];
   irnode_t *n;
   int cnt, total;

   (void) ctx;

   n = tree_dat(t);
   cnt = sprintf(buf, "%d", lv);
   buf[cnt] = '\0';
   total = lv * strlen("  ");
   total -= cnt;

   fputs(buf, stdout);
   for (int i = 0; i < total; i++)
      putchar(' ');

   printf("[%s] = [", resolve_nodekind(n->kind));
   switch (n->kind) {
      case IrnodekindVar:
         printf("%s]", resolve_var(n->dat.ui));
      break;

      case IrnodekindOpcode:
         printf("%s]", resolve_opcode(n->dat.ui));
      break;

      default:
         switch (n->datkind) {
            case IrnodeDatkindInt:
               printf("%d]", n->dat.i);
            break;

            case IrnodeDatkindUint:
               printf("%u]", n->dat.ui);
            break;

            case IrnodeDatkindStr:
               printf("%s]",
                  n->dat.s.len
                  ? (char *) n->dat.s.run
                  : Cbblack "(empty)" Creset
               );
               printf(" " Cbblack "(len = %d)" Creset "", n->dat.s.len);
            break;

            default: ;
         }
   }
   if (n->kind != IrnodekindOpcode) {
      putchar('\n');
      return;
   }
   printf(" " Cbblack "at " Ccyan "code+0x%08" SPL_UINT_FMTSPC_HEX Creset "\n", n->offset);
}

static const char *resolve_nodekind(irnodekind_t kind) {
   switch (kind) {
      case IrnodekindEop     : return "END OF PROGRAM";
      case IrnodekindRoot    : return "ROOT";
      case IrnodekindDp      : return "DP";
      case IrnodekindNrtv    : return "NRTV";
      case IrnodekindAct     : return "ACT";
      case IrnodekindScene   : return "SCENE";
      case IrnodekindBlock   : return "BLOCK";
      case IrnodekindOpcode  : return "OPCODE";
      case IrnodekindVar     : return "VARIABLE";
      case IrnodekindPerson  : return "PERSON";
      case IrnodekindConst   : return "CONST";
      case IrnodekindData    : return "DATA";
      default: return NULL;
   }
}
