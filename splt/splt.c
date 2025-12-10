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
      "loaded the source file \033[0;32m%s\033[0m (total %d lines, %d chars)\n",
      ov.src, lc, wc);

   // Main logic
   toks = lex(&of, &ov, lc);
   fmtwrt(ENPREFIX "scanning done (total %d tokens)\n", arr_size(toks));
   // arr_foreach(toks, &print_token);

   pt = parse(&of, &ov, toks);
   fmtwrt(ENPREFIX "parsing done (total %d nodes)\n", count_tree_node(pt));
   // tree_pre_traverse(pt, &print_node, 0);

   // Cleanup
   tree_post_traverse(pt, &cleanup_node, 0);
   tree_prune(pt);
   unloadfl(ls, lc);

   return 0;
}

static void cleanup_node(void *data, int _) {
   (void) _;
   free(((node_t *) data)->run);
}

static void print_token(void *dat, int idx) {
   printf("idx = [%d], token = [%s]\n", idx, ((token_t *) dat)->run);
}

static void print_node(void *data, int lv) {
   static char buf[128];

   node_t *n;
   int cnt, total;

   n = data;
   cnt = sprintf(buf, "%d", lv);
   buf[cnt] = '\0';
   total = lv * strlen("  ");
   total -= cnt;

   fputs(buf, stdout);
   for (int i = 0; i < total; i++)
      putchar(' ');

   printf("[%s] = [%s]\n", n->tag,
      n->len ? (char *) n->run : Cbblack "(empty)" Creset);
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
