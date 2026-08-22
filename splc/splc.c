#include "splc.h"
#include "splc.internals.h"

int main(int argc, const char *argv[]) {
   compile_ctx_t cctx;

   cctx.argc = argc;
   cctx.argv = argv;

   /* Initialization */
   init_msg();
   parse_args(&cctx);
   process_opts(&cctx);

   cctx.ls = loadfile(cctx.ov->src, &cctx.lc, &cctx.wc);
   safe_vprintf(ENPREFIX
      "loaded the source file " Cbyellow "%s" Creset
      " (total " Cbwhite "%d" Creset " lines, " Cbwhite "%d" Creset " chars)\n",
      cctx.ov->src, cctx.lc, cctx.wc);

   dbload();

   /* MAIN LOGIC */
   safe_fputs(stdout, ENPREFIX "scanning...");
   lex(&cctx);
   safe_vprintf(" " Cgreen "done!" Creset
      "\t(total " Cbwhite "%zu" Creset " tokens)\n",
      array_size(cctx.toks)
   );
   // array_foreach(cctx.toks, print_token);

   safe_fputs(stdout, ENPREFIX "parsing...");
   parse(&cctx);
   safe_vprintf(" " Cgreen "done!" Creset
      "\t(total " Cbwhite "%zu" Creset " nodes)\n",
      count_tree_node(cctx.pt)
   );
   // tree_pre_traverse(cctx.pt, print_node, 0, NULL);

   safe_fputs(stdout, ENPREFIX "type-checking...");
   typecheck(&cctx);
   safe_vprintf(" " Cgreen "done!" Creset "\n");
   // tree_pre_traverse(cctx.pt, print_node, 0, NULL);

   safe_fputs(stdout, ENPREFIX "context-checking...");
   ctxcheck(&cctx);
   safe_vprintf(" " Cgreen "done!" Creset "\n");
   // tree_pre_traverse(cctx.pt, print_node, 0, NULL);

   safe_fputs(stdout, ENPREFIX "generating IR...");
   irgenerate(&cctx);
   safe_vprintf(" " Cgreen "done!" Creset
      "\t(total " Cbwhite "%zu" Creset " nodes)\n",
      count_opcodes(cctx.irt)
   );
   // tree_pre_traverse(cctx.irt, debug_print_irnode, 0, NULL);

   if (cctx.of->opt) {
      safe_fputs(stdout, ENPREFIX "optimizing IR...");
      iroptimize(&cctx);
      safe_vprintf(" " Cgreen "done!" Creset
         "\t(total " Cbwhite "%zu" Creset " nodes)\n",
         count_opcodes(cctx.irt)
      );
      // tree_pre_traverse(cctx.irt, debug_print_irnode, 0, NULL);
   }

   if (cctx.of->dmp) irdump(&cctx);

   if (cctx.of->tgt) {
      transpile2c(&cctx);
   }
   else {
      assemble(&cctx);
      // tree_pre_traverse(cctx.irt, debug_print_irnode, 0, NULL);
   }

   // safe_fputs(stdout, ENPREFIX "transpiling...");
   // transpile(&of, &ov);
   // safe_vprintf(" " Cgreen "done!" Creset "\n");

   /* Cleanup */
   destroy_irt(&cctx);
   destroy_pt(&cctx);
   array_destroy(cctx.toks);
   dbunload();
   array_destroy(cctx.ls);
   destroy_options(&cctx);

   return 0;
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
