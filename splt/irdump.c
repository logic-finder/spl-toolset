#include "irdump.h"
#include "irdump.internals.h"

extern void irdump(void) {
   char *destname;

   destname = make_destname("hello.spl");
   fp = sfopen(destname, "w");

   debug = false;  /* emit debugging data? */
   tree_pre_traverse(irt, route, 0, NULL);

   sfclose(fp);
   free(destname);
}

static char *make_destname(const char *orig) {
   char *temp, *buf;
   size_t tempsiz;

   temp = extfnm(orig, false);
   tempsiz = strlen(temp);
   buf = smalloc(tempsiz + strlen(IR_EXTENSION) + 1);
   strcpy(buf, temp);
   strcat(buf, IR_EXTENSION);

   return buf;
}

static void route(tree_t *t, int lv, void *ctx) {
   irnode_t *irn;

   (void) lv, (void) ctx;
   irn = tree_dat(t);

   switch (irn->kind) {
      case IrnodekindOpcode : handle_opcode(t); break;
      case IrnodekindAct    : handle_act   (t); break;
      case IrnodekindScene  : handle_scene (t); break;
      case IrnodekindBlock  : handle_block (t); break;
      default: ;
   }
}

static void handle_opcode(tree_t *t) {
   irnode_t *irn;

   irn = tree_dat(t);

   switch (irn->dat.n) {
      case IropcodeSet    : handle_set      (t); break;
      case IropcodeEnter  : /* fall-through */
      case IropcodeExit   : /* fall-through */
      case IropcodeSpeak  : handle_enterlike(t); break;
      case IropcodeOutN   : /* fall-through */
      case IropcodeOutC   : /* fall-through */
      case IropcodeInN    : /* fall-through */
      case IropcodeInC    : /* fall-through */
      case IropcodeRememb : /* fall-through */
      case IropcodeRecall : /* fall-through */
      case IropcodeNegate : /* fall-through */
      case IropcodeExeunt : handle_paramless_opcode(t); break;
      case IropcodePush   : /* fall-through */
      case IropcodePop    : handle_pushlike(t); break;
      case IropcodeSum    : /* fall-through */
      case IropcodeDiff   : /* fall-through */
      case IropcodeProd   : /* fall-through */
      case IropcodeQuot   : /* fall-through */
      case IropcodeRem    : handle_binary_op(t); break;
      case IropcodeSqrt   : /* fall-through */
      case IropcodeSqur   : /* fall-through */
      case IropcodeCube   : /* fall-through */
      case Iropcode2x     : /* fall-through */
      case IropcodeFact   : /* fall-through */
      case IropcodeEq     : /* fall-through */
      case IropcodeGt     : /* fall-through */
      case IropcodeLt     : handle_unary_op(t); break;
      case IropcodeGoto   : handle_goto(t); break;
      case IropcodeJumpT  : /* fall-through */
      case IropcodeJumpF  : handle_jumplike(t); break;
   }
}

static void handle_act(tree_t *t) {
   irnode_t *n = tree_dat(t);
   curr_act = n->dat.s.run;
}

static void handle_scene(tree_t *t) {
   irnode_t *n = tree_dat(t);
   if (debug) emit_debug_data(n);
   ffmtwrt(
      fp,
      "\nAct_%s_Scene_%s:\n",
      curr_act,
      n->dat.s.run
   );
}

static void handle_block(tree_t *t) {
   irnode_t *n = tree_dat(t);
   if (!n->dat.n)
      return;
   ffmtwrt(
      fp,
      "\n.L%d:\n",
      n->dat.n
   );
}

static void handle_set(tree_t *t) {
   irnode_t *n, *p1, *p2;

   n = tree_dat(t);
   p1 = tree_chdat(t, 0);
   p2 = tree_chdat(t, 1);

   if (debug) emit_debug_data(n);

   switch (p2->kind) {
      case IrnodekindPerson : goto person;
      case IrnodekindVar    : goto var;
      default: goto common;
   }

person:
   ffmtwrt(
      fp,
      "%s%s %s dp[%d]\n",
      INDENT,
      resolve_opcode(n->dat.n),
      resolve_var(p1->dat.n),
      p2->dat.n
   );
   return;

var:
   ffmtwrt(
      fp,
      "%s%s %s %s\n",
      INDENT,
      resolve_opcode(n->dat.n),
      resolve_var(p1->dat.n),
      resolve_var(p2->dat.n)
   );
   return;

common:
   ffmtwrt(
      fp,
      "%s%s %s %d\n",
      INDENT,
      resolve_opcode(n->dat.n),
      resolve_var(p1->dat.n),
      p2->dat.n
   );
}

static void handle_enterlike(tree_t *t) {
   irnode_t *n, *p1;

   n = tree_dat(t);
   p1 = tree_chdat(t, 0);

   if (debug) emit_debug_data(n);
   ffmtwrt(
      fp,
      "%s%s %d\n",
      INDENT,
      resolve_opcode(n->dat.n),
      p1->dat.n
   );
}

static void handle_pushlike(tree_t *t) {
   irnode_t *n, *p1;

   n = tree_dat(t);
   p1 = tree_chdat(t, 0);

   ffmtwrt(
      fp,
      "%s%s %s\n",
      INDENT,
      resolve_opcode(n->dat.n),
      resolve_var(p1->dat.n)
   );
}

static void handle_goto(tree_t *t) {
   irnode_t *n, *p1, *p2, *act_dat;
   tree_t *act;

   n = tree_dat(t);
   p1 = tree_chdat(t, 0);
   p2 = tree_chdat(t, 1);

   if (debug) emit_debug_data(n);

   if (p1->dat.n == NODEKIND_ACT) {
      ffmtwrt(
         fp,
         "%s%s Act_%s_Scene_I\n",
         INDENT,
         resolve_opcode(n->dat.n),
         p2->dat.s.run
      );
      return;
   }

   /* Handles == NODEKIND_SCENE */
   act = tree_parent(tree_parent(t));
   act_dat = tree_dat(act);

   ffmtwrt(
      fp,
      "%s%s Act_%s_Scene_%s\n",
      INDENT,
      resolve_opcode(n->dat.n),
      act_dat->dat.s.run,
      p2->dat.s.run
   );
}

static void handle_jumplike(tree_t *t) {
   irnode_t *n, *p1;

   n = tree_dat(t);
   p1 = tree_chdat(t, 0);

   if (debug) emit_debug_data(n);
   ffmtwrt(
      fp,
      "%s%s .L%d\n",
      INDENT,
      resolve_opcode(n->dat.n),
      p1->dat.n
   );
}

static void handle_binary_op(tree_t *t) {
   irnode_t *n, *p1, *p2, *p3;

   n = tree_dat(t);
   p1 = tree_chdat(t, 0);
   p2 = tree_chdat(t, 1);
   p3 = tree_chdat(t, 2);

   if (debug) emit_debug_data(n);
   ffmtwrt(
      fp,
      "%s%s %s %s %s\n",
      INDENT,
      resolve_opcode(n->dat.n),
      resolve_var(p1->dat.n),
      resolve_var(p2->dat.n),
      resolve_var(p3->dat.n)
   );
}

static void handle_unary_op(tree_t *t) {
   irnode_t *n, *p1, *p2;

   n = tree_dat(t);
   p1 = tree_chdat(t, 0);
   p2 = tree_chdat(t, 1);

   if (debug) emit_debug_data(n);
   ffmtwrt(
      fp,
      "%s%s %s %s\n",
      INDENT,
      resolve_opcode(n->dat.n),
      resolve_var(p1->dat.n),
      resolve_var(p2->dat.n)
   );
}

static void handle_paramless_opcode(tree_t *t) {
   irnode_t *n = tree_dat(t);
   if (debug) emit_debug_data(n);
   ffmtwrt(
      fp,
      "%s%s\n",
      INDENT,
      resolve_opcode(n->dat.n)
   );
}

static inline void emit_debug_data(irnode_t *n) {
   ffmtwrt(fp, "; %d:%d\n", n->lnum, n->lpos);
}

static const char *resolve_opcode(iropcode_t opcode) {
   switch (opcode) {
      case IropcodeSet     : return "SET";
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
