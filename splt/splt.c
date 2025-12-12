#include "splt.type.h"

arr_t *ls;  /* External Variable */

int main(int argc, const char **argv) {
   optflg_t of = {0};
   optval_t ov;
   int lc, wc;
   arr_t *toks;
   tree_t *pt;

   // Initialize
   init_msg();
   parse_args(argv, &of, &ov);
   process_opts(argc, &of, &ov);

   ls = loadfile(ov.src, &lc, &wc);
   fmtwrt(ENPREFIX
      "loaded the source file \033[0;32m%s\033[0m (total " Cbwhite "%d" Creset " lines, " Cbwhite "%d" Creset " chars)\n",
      ov.src, lc, wc);

   // Main logic
   toks = lex(&of, &ov, lc);
   fmtwrt(ENPREFIX "scanning done (total " Cbwhite "%d" Creset " tokens)\n", arr_size(toks));
   // arr_foreach(toks, print_token);

   pt = parse(&of, &ov, toks);
   fmtwrt(ENPREFIX "parsing done (total " Cbwhite "%d" Creset " nodes)\n", count_tree_node(pt));
   tree_pre_traverse(pt, print_node, 0);

   //typecheck(&of, &ov, pt);
   //tree_pre_traverse(pt, print_node, 0);

   // Cleanup
   tree_post_traverse(pt, cleanup_node, 0);
   tree_prune(pt);
   unloadfl(ls, lc);

   return 0;
}

static void cleanup_node(tree_t *t, int _) {
   (void) _;
   free(((node_t *) tree_dat(t))->run);
}

static void print_token(void *dat, int idx) {
   printf("idx = [%d], token = [%s]\n", idx, ((token_t *) dat)->run);
}

static void print_node(tree_t *t, int lv) {
   static char buf[128];

   node_t *n;
   int cnt, total;

   n = tree_dat(t);
   cnt = sprintf(buf, "%d", lv);
   buf[cnt] = '\0';
   total = lv * strlen("  ");
   total -= cnt;

   fputs(buf, stdout);
   for (int i = 0; i < total; i++)
      putchar(' ');

   printf("[%s] = [%s]", nodekind2str(n->kind),
      n->len ? (char *) n->run : Cbblack "(empty)" Creset);
   printf(" " Cbblack "(len = %d)" Creset "\n", n->len);
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

static const char *nodekind2str(nodekind_t kind) {
   switch (kind) {
      case NODEKIND_ROOT   : return "ROOT";
      case NODEKIND_DATA   : return "DATA";
      case NODEKIND_ADJ    : return "ADJ";
      case NODEKIND_NOUN   : return "NOUN";
      case NODEKIND_AFFIRM : return "AFFIRM";
      case NODEKIND_NEGATE : return "NEGATE";
      case NODEKIND_LHS    : return "LHS";
      case NODEKIND_RHS    : return "RHS";
      case NODEKIND_PERSON : return "PERSON";
      case NODEKIND_EQ     : return "EQ";
      case NODEKIND_INEQ   : return "INEQ";
      case NODEKIND_TITLE  : return "TITLE";
      case NODEKIND_DP     : return "DP";
      case NODEKIND_CHAR   : return "CHAR";
      case NODEKIND_ACT    : return "ACT";
      case NODEKIND_SCENE  : return "SCENE";
      case NODEKIND_ENTER  : return "ENTER";
      case NODEKIND_EXIT   : return "EXIT";
      case NODEKIND_EXEUNT : return "EXEUNT";
      case NODEKIND_LINE   : return "LINE";
      case NODEKIND_ASSIGN : return "ASSIGN";
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
