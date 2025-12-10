#include "typecheck.h"
#include "typecheck.type.h"

static const node_t *enode;

extern void typecheck(optflg_t *of, optval_t *ov, tree_t *pt) {
   // Preprocess names
   tree_post_traverse(pt, coalesce_name, 0);

   // Type-check
   // TODO: dp 다음원소부터 돌리는게 날듯
   // TODO: 사람이름, 비교급
   tree_post_traverse(pt, typecheck_adj, 0);
   //tree_post_traverse(pt, typecheck_noun, 0);
}

static void coalesce_name(tree_t *t, int _) {
   node_t *n;
   char *buf;
   int i, clen, rlen;

   (void) _;
   n = tree_dat(t);
   if (strcmp(n->tag, "CHAR"))
      return;

   clen = tree_clen(t);
   rlen = 0;
   for (i = 0; i < clen; i++)
      // notice that len == strlen(run) + 1
      rlen += TREE_CHDAT(t, i)->len;

   /*
    * input  = ["the", "misty", "Ghost"]
    * output = "the_misty_Ghost00"
    *    where _ = space and 0 = \0
    */
   rlen++;
   buf = smalloc(rlen);
   buf[0] = '\0';
   for (i = 0; i < clen; i++) {
      strcat(buf, TREE_CHDAT(t, i)->run);
      strcat(buf, " ");
   }
   buf[rlen - 2] = '\0';

   n->run = buf;
   n->len = rlen;
}

static void typecheck_adj(tree_t *t, int _) {
   node_t *n;
   bool ret;
   int val;

   (void) _;
   n = tree_dat(t);
   if (strcmp(n->tag, "ADJ"))
      return;

   ret = query_adj(n->run, &val);
   if (!ret) {
      reason = msgs.err.sem.bad_adj;
      enode = n;
      semerr();
   }
}

static void typecheck_noun(tree_t *t, int _) {
   // TODO: 사람이름, 대명사같은거 고려해야함
}

static inline void semerr(void) {
   err_template(tell, Cbred, "<semantic error> ");
}

static void tell(void) {
   int lnum, lpos;
   line_t *l;

   lnum = enode->lnum;
   lpos = enode->lpos;
   l = arr_peek(ls, lnum - 1);

   ffmtwrt(stderr,
      "%s " Cbcyan "%s" Creset "\n"
      "[%s:%d:%d] " Cbwhite "note:" Creset " problematic at here\n"
      "%4d|%.*s" Cbblue "%s" Creset "\n",
      reason, enode->run,
      sfname, lnum, lpos,
      lnum, lpos - 1, l->run, &l->run[lpos - 1]
   );
}
