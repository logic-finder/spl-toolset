#include "tree.adt.h"
#include "tree.adt.internals.h"

extern tree_t *tree_plant(void *data, size_t dsiz) {
   tree_t *ret;  /* a little sapling! */

   ret = smalloc(sizeof *ret);
   ret->children = smalloc(INIT_CMAX * sizeof *ret->children);
   if (data)
      ret->dat = smalloc(dsiz);
   else
      ret->dat = NULL;

   ret->parent = NULL;
   ret->cmax = INIT_CMAX;
   ret->clen = 0;
   ret->siz = dsiz;
   memcpy(ret->dat, data, dsiz);

   return ret;
}

extern tree_t *tree_graft(
   tree_t * restrict base,
   tree_t * restrict in
) {
   if (tree_full(base))
      tree_enlarge(base);

   in->parent = base;
   base->children[base->clen++] = in;

   return in;
}

static bool tree_full(const tree_t *t) {
   return t->cmax == t->clen;
}

static void tree_enlarge(tree_t *t) {
   t->cmax *= 2;
   t->children = srealloc_arr(
      t->children,
      t->cmax,
      ESIZ(t->children)
   );
}

extern void tree_prune(tree_t *t) {
   /* exit condition */
   if (t->clen == 0) {
      tree_prune_cb(t);
      return;
   }

   for (int i = 0; i < t->clen; i++)
      tree_prune(t->children[i]);

   tree_prune_cb(t);
}

static void tree_prune_cb(tree_t *t) {
   free(t->children);
   free(t->dat);
   free(t);
}

extern void tree_pre_traverse(tree_t *t, tree_callback_t *cb, int lv, void *ctx) {
   (*cb)(t, lv, ctx);
   if (!t->clen) return;
   for (int i = 0; i < t->clen; i++)
      tree_pre_traverse(t->children[i], cb, lv + 1, ctx);
}

extern void tree_post_traverse(tree_t *t, tree_callback_t *cb, int lv, void *ctx) {
   if (!t->clen) {
      (*cb)(t, lv, ctx);
      return;
   }
   for (int i = 0; i < t->clen; i++)
      tree_post_traverse(t->children[i], cb, lv + 1, ctx);
   (*cb)(t, lv, ctx);
}

extern int tree_clen(const tree_t *t) {
   return t->clen;
}

extern void *tree_dat(const tree_t *t) {
   return t->dat;
}

extern tree_t *tree_child(const tree_t *t, int idx) {
   return t->children[idx];
}

extern void *tree_chdat(const tree_t *t, int idx) {
   return t->children[idx]->dat;
}

extern tree_t *tree_parent(const tree_t *t) {
   return t->parent;
}
