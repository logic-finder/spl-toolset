#include <ctype.h>
#include <string.h>
#include "msg.h"
#include "lex.h"
#include "parse.h"
#include "global.h"
#include "wrapper.h"
#include "loadfile.h"
#include "argparse.h"
#include "colorcode.h"

static void print_token(void *data, int _);
static void traverse(tree_t *base, int level);
static int count_tree_node(tree_t *root);

int main(int argc, const char **argv) {
   optflg_t of = {0};
   optval_t ov;
   line_t *lines;
   int lc, wc;
   list_t *toks;
   tree_t *pt;

   // Init
   init_msg();
   parse_args(argv, &of, &ov);
   lines = loadfile(ov.src, &lc, &wc);

   fmtwrt(ENPREFIX
      "loaded the source file \033[0;32m%s\033[0m (total %d lines, %d chars)\n",
      ov.src, lc, wc);

   // Main logic
   toks = lex(&of, &ov, lines, lc);
   fmtwrt(ENPREFIX "scanning done (total %d tokens)\n", toks->size);
   //list_foreach(toks, print_token);
   pt = parse(&of, &ov, toks);
   fmtwrt(ENPREFIX "parsing done (total %d nodes)\n", count_tree_node(pt));
   traverse(pt, 0);

   // Cleanup
   tree_prune(pt);
   unloadfl(lines, lc);

   return 0;
}

static void print_token(void *data, int idx) {
   printf("idx = [%d], token = [%s]\n", idx, ((node_t *) data)->data);
}

static void traverse(tree_t *base, int level) {
   //static int level = 0;

   for (int i = 0; i < level; i++)
      fputs("  ", stdout);
   printf("[%s] = [%s]\n", base->tag,
      base->dsiz ? (char *) base->data : Cbblack "(empty)" Creset);

   if (base->clen == 0)
      return;  /* end of recursion */

   //level++;
   for (int i = 0; i < base->clen; i++)
      traverse(base->child[i], level + 1);
}

static int count_tree_node(tree_t *root) {
   int cnt = 0;

   if (root->clen == 0)
      return 1;

   for (int i = 0; i < root->clen; i++)
      cnt += count_tree_node(root->child[i]);

   return cnt + 1;
}
