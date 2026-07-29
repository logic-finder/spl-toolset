#include "splt.internals.h"

int main(int argc, const char **argv) {
   optflg_t of = {0};
   optval_t ov;
   arr_t *toks;
   int lc, wc;

   // Initialize
   init_msg();
   parse_args(argv, &of, &ov);
   process_opts(argc, &of, &ov);

   ls = loadfile(ov.src, &lc, &wc);
   fmtwrt(ENPREFIX
      "loaded the source file " Cgreen "%s" Creset
      " (total " Cbwhite "%d" Creset " lines, " Cbwhite "%d" Creset " chars)\n",
      ov.src, lc, wc);

   dbload();

   // Main logic
   sfputs(stdout, ENPREFIX "scanning...");
   toks = lex(&of, &ov, lc);
   fmtwrt(" " Cbgreen "done!" Creset
      "\t(total " Cbwhite "%d" Creset " tokens)\n",
      arr_size(toks)
   );
   // arr_foreach(toks, print_token);

   sfputs(stdout, ENPREFIX "parsing...");
   pt = parse(&of, &ov, toks);
   fmtwrt(" " Cbgreen "done!" Creset
      "\t(total " Cbwhite "%d" Creset " nodes)\n",
      count_tree_node(pt)
   );
   // tree_pre_traverse(pt, print_node, 0, NULL);

   sfputs(stdout, ENPREFIX "type-checking...");
   typecheck(&of, &ov);
   fmtwrt(" " Cbgreen "done!" Creset "\n");
   // tree_pre_traverse(pt, print_node, 0, NULL);

   sfputs(stdout, ENPREFIX "context-checking...");
   ctxcheck(&of, &ov);
   fmtwrt(" " Cbgreen "done!" Creset "\n");
   // tree_pre_traverse(pt, print_node, 0, NULL);

   sfputs(stdout, ENPREFIX "generating IR...");
   irgenerate();
   fmtwrt(" " Cbgreen "done!" Creset
      "\t(total " Cbwhite "%zu" Creset " tokens)\n",
      count_opcodes(irt)
   );
   // tree_pre_traverse(irt, print_irnode, 0, NULL);

   // (optional) optimizing IR...

   sfputs(stdout, ENPREFIX "dumping IR...");
   irdump();
   fmtwrt(" " Cbgreen "done!" Creset "\n");

   // or compiling...
   // sfputs(stdout, ENPREFIX "transpiling...");
   // transpile(&of, &ov);
   // fmtwrt(" " Cbgreen "done!" Creset "\n");

   // Cleanup
   tree_post_traverse(pt, cleanup_node, 0, NULL);
   tree_prune(pt);
   unloadfl(ls, lc);
   dbunload();

   return 0;
}

static void cleanup_node(tree_t *t, int lv, void *ctx) {
   node_t *n;

   (void) lv, (void) ctx;
   n = tree_dat(t);
   if (n->datkind != DATKIND_STR)
      return;
   free(n->dat.s.run);
}

static void print_token(void *dat, int idx) {
   printf("idx = [%d], token = [%s]\n", idx, ((token_t *) dat)->run);
}

static void print_node(tree_t *t, int lv, void *ctx) {
   static char buf[128];

   node_t *n;
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

   printf("[%s] = [", nodekind2str(n->kind));
   if (n->datkind == DATKIND_INT)
      printf("%d]\n", n->dat.n);
   else {
      printf("%s]",
         n->dat.s.len
         ? (char *) n->dat.s.run
         : Cbblack "(empty)" Creset
      );
      printf(" " Cbblack "(len = %d)" Creset "\n", n->dat.s.len);
   }
}

static int count_tree_node(tree_t *root) {
   int cnt, clen;

   cnt = 0;
   clen = tree_clen(root);

   if (clen == 0)
      return 1;

   for (int i = 0; i < clen; i++)
      cnt += count_tree_node(tree_child(root, i));

   return cnt + 1;
}

static size_t count_opcodes(tree_t *irt) {
   int counter = 1;  /* +1 for SET dpsz n */
   tree_pre_traverse(irt, opcode_counter, 0, &counter);
   return counter;
}

static void opcode_counter(tree_t *t, int lv, void *ctx) {
   irnode_t *n;
   int *counter;

   (void) lv;
   n = tree_dat(t);
   if (n->kind != IrnodekindBlock)
      return;
   counter = ctx;
   *counter += tree_clen(t);
}

static const char *nodekind2str(nodekind_t kind) {
   switch (kind) {
      case NODEKIND_ROOT   : return "ROOT";
      case NODEKIND_DATA   : return "DATA";
      case NODEKIND_ROMNUM : return "ROMAN_NUMERAL";
      case NODEKIND_SUBJ   : return "SUBJECT";
      case NODEKIND_CONST  : return "CONST";
      case NODEKIND_ADJ    : return "ADJ";
      case NODEKIND_NOUN   : return "NOUN";
      case NODEKIND_PNOUN  : return "NOUN_POS";
      case NODEKIND_NNOUN  : return "NOUN_NEG";
      case NODEKIND_AFFIRM : return "AFFIRM";
      case NODEKIND_NEGATE : return "NEGATE";
      case NODEKIND_CONSEQ : return "CONSEQUENT";
      case NODEKIND_LHS    : return "LHS";
      case NODEKIND_RHS    : return "RHS";
      case NODEKIND_P1     : return "PERSON_1ST";
      case NODEKIND_P2     : return "PERSON_2ND";
      case NODEKIND_P3     : return "PERSON_3RD";
      case NODEKIND_EQ     : return "EQ";
      case NODEKIND_INEQ   : return "INEQ";
      case NODEKIND_GT     : return "INEQ_GT";
      case NODEKIND_LT     : return "INEQ_LT";
      case NODEKIND_TITLE  : return "TITLE";
      case NODEKIND_DP     : return "DP";
      case NODEKIND_CHDECL : return "CHARDECL";
      case NODEKIND_NRTV   : return "NARRATIVE";
      case NODEKIND_CHAR   : return "CHAR";
      case NODEKIND_ACT    : return "ACT";
      case NODEKIND_SCENE  : return "SCENE";
      case NODEKIND_ENTER  : return "ENTER";
      case NODEKIND_EXIT   : return "EXIT";
      case NODEKIND_EXEUNT : return "EXEUNT";
      case NODEKIND_LINE   : return "LINE";
      case NODEKIND_ASGN1  : return "ASSIGN_A";
      case NODEKIND_ASGN2  : return "ASSIGN_B";
      case NODEKIND_OUT_N  : return "OUT_N";
      case NODEKIND_OUT_C  : return "OUT_C";
      case NODEKIND_IN_N   : return "IN_N";
      case NODEKIND_IN_C   : return "IN_C";
      case NODEKIND_GOTO   : return "GOTO";
      case NODEKIND_COND   : return "COND";
      case NODEKIND_IF     : return "IF";
      case NODEKIND_PUSH   : return "PUSH";
      case NODEKIND_POP    : return "POP";
      case NODEKIND_SUM    : return "SUM";
      case NODEKIND_DIFF   : return "DIFF";
      case NODEKIND_PROD   : return "PROD";
      case NODEKIND_QUOT   : return "QUOT";
      case NODEKIND_REM    : return "REM";
      case NODEKIND_SQRT   : return "SQRT";
      case NODEKIND_SQUR   : return "SQUR";
      case NODEKIND_CUBE   : return "CUBE";
      case NODEKIND_2X     : return "2X";
      case NODEKIND_FACT   : return "FACT";
      default : goto unreachable;
   }
   unreachable: return NULL;
}

static void print_irnode(tree_t *t, int lv, void *ctx) {
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

   printf("[%s] = [", irnodekind2str(n->kind));
   switch (n->kind) {
      case IrnodekindVar:
         printf("%s]\n", irnodekindvar2str(n->dat.n));
      break;

      case IrnodekindOpcode:
         printf("%s]\n", irnodekindopcode2str(n->dat.n));
      break;

      default:
         if (n->datkind == IrnodeDatkindInt)
            printf("%d]\n", n->dat.n);
         else {
            printf("%s]",
               n->dat.s.len
               ? (char *) n->dat.s.run
               : Cbblack "(empty)" Creset
            );
            printf(" " Cbblack "(len = %d)" Creset "\n", n->dat.s.len);
         }
   }
}

static const char *irnodekind2str(irnodekind_t kind) {
   switch (kind) {
      case IrnodekindUnknown : return "__UNKNOWN__";
      case IrnodekindRoot    : return "ROOT";
      case IrnodekindAct     : return "ACT";
      case IrnodekindScene   : return "SCENE";
      case IrnodekindBlock   : return "BLOCK";
      case IrnodekindOpcode  : return "INSTRUCTION";
      case IrnodekindVar     : return "VARIABLE";
      case IrnodekindPerson  : return "PERSON";
      case IrnodekindConst   : return "CONST";
      case IrnodekindData    : return "DATA";
      default: return NULL;
   }
}

static const char *irnodekindvar2str(irvar_t var) {
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

static const char *irnodekindopcode2str(iropcode_t opcode) {
   switch (opcode) {
      case IropcodeUnknown : return "__UNKNOWN__";
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
      case Iropcode2x      : return "2x";
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
