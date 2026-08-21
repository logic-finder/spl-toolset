#include "splc.internals.h"

int main(int argc, const char *argv[]) {
   optflg_t of = {0};
   optval_t ov;
   array_t *toks;
   size_t lc, wc;

   /* Initialization */
   init_msg();
   parse_args(argv, &of, &ov);
   process_opts(argc, &of, &ov);

   ls = loadfile(ov.src, &lc, &wc);
   safe_vprintf(ENPREFIX
      "loaded the source file " Cbyellow "%s" Creset
      " (total " Cbwhite "%d" Creset " lines, " Cbwhite "%d" Creset " chars)\n",
      ov.src, lc, wc);

   dbload();

   /* MAIN LOGIC */
   safe_fputs(stdout, ENPREFIX "scanning...");
   toks = lex(&of, &ov, ls, lc);
   safe_vprintf(" " Cgreen "done!" Creset
      "\t(total " Cbwhite "%zu" Creset " tokens)\n",
      array_size(toks)
   );
   // array_foreach(toks, print_token);

   safe_fputs(stdout, ENPREFIX "parsing...");
   pt = parse(&of, &ov, toks);
   safe_vprintf(" " Cgreen "done!" Creset
      "\t(total " Cbwhite "%zu" Creset " nodes)\n",
      count_tree_node(pt)
   );
   // tree_pre_traverse(pt, print_node, 0, NULL);

   safe_fputs(stdout, ENPREFIX "type-checking...");
   typecheck(&of, &ov);
   safe_vprintf(" " Cgreen "done!" Creset "\n");
   // tree_pre_traverse(pt, print_node, 0, NULL);

   safe_fputs(stdout, ENPREFIX "context-checking...");
   ctxcheck(&of, &ov);
   safe_vprintf(" " Cgreen "done!" Creset "\n");
   // tree_pre_traverse(pt, print_node, 0, NULL);

   safe_fputs(stdout, ENPREFIX "generating IR...");
   irgenerate();
   safe_vprintf(" " Cgreen "done!" Creset
      "\t(total " Cbwhite "%zu" Creset " nodes)\n",
      count_opcodes(irt)
   );
   // tree_pre_traverse(irt, debug_print_irnode, 0, NULL);

   if (of.opt) {
      safe_fputs(stdout, ENPREFIX "optimizing IR...");
      iroptimize();
      safe_vprintf(" " Cgreen "done!" Creset
         "\t(total " Cbwhite "%zu" Creset " nodes)\n",
         count_opcodes(irt)
      );
      // tree_pre_traverse(irt, debug_print_irnode, 0, NULL);
   }

   if (1) irdump();

   assemble();
   // tree_pre_traverse(irt, debug_print_irnode, 0, NULL);

   transpile2c();

   // safe_fputs(stdout, ENPREFIX "transpiling...");
   // transpile(&of, &ov);
   // safe_vprintf(" " Cgreen "done!" Creset "\n");

   /* Cleanup */
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

static size_t count_tree_node(tree_t *root) {
   size_t cnt, clen;

   cnt = 0;
   clen = tree_clen(root);

   if (clen == 0)
      return 1;

   for (size_t i = 0; i < clen; i++)
      cnt += count_tree_node(tree_child(root, i));

   return cnt + 1;
}

static size_t count_opcodes(tree_t *irt) {
   size_t counter = 0;
   tree_pre_traverse(irt, opcode_counter, 0, &counter);
   return counter;
}

static void opcode_counter(tree_t *t, int lv, void *ctx) {
   irnode_t *n;
   size_t *counter;

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
      case NODEKIND_SUBJ   : return "SUBJECT"; // fixme: in use?
      case NODEKIND_CONST  : return "CONST";
      case NODEKIND_ADJ    : return "ADJ";
      case NODEKIND_ZERO   : return "NOUN_ZERO";
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
      case NODEKIND_ASGN3  : return "ASSIGN_C";
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
      default: return NULL;   /* unreachable */
   }
}
