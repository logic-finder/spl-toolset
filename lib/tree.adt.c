#include "tree.adt.h"
#include "tree.adt.internals.h"

// TODO: introuduce destructor

extern tree_t *tree_plant(void *data, size_t dsiz, arena_t *arena) {
   tree_t *ret;  /* a little sapling! */

   if (!(ret = arena_malloc_fallback(arena, sizeof *ret))) {
      return NULL;
   }

   ret->children = arena_malloc_fallback(
      arena,
      INIT_CMAX * ESIZ(ret->children)
   );
   if (!ret->children) {
      goto err1;
   }
   ret->parent = NULL;
   ret->cmax = INIT_CMAX;
   ret->clen = 0;
   ret->siz = dsiz;
   ret->arena = arena;

   if (data) {
      if (!(ret->dat = arena_malloc_fallback(arena, dsiz))) {
         goto err2;
      }
      memcpy(ret->dat, data, dsiz);
   }
   else ret->dat = NULL;
   return ret;

err2:
   arena_free_fallback(arena, ret->children);
err1:
   arena_free_fallback(arena, ret);
   return NULL;
}

extern tree_t *tree_graft(
   tree_t * restrict base,
   tree_t * restrict in
) {
   if (tree_full(base)) {
      if (!tree_enlarge(base)) {
         return NULL;
      }
   }

   in->parent = base;
   base->children[base->clen++] = in;

   return in;
}

static bool tree_full(const tree_t *t) {
   return t->cmax == t->clen;
}

static bool tree_enlarge(tree_t *t) {
   t->children = arena_realloc2x_arr_fallback(
      t->arena,
      t->children,
      &t->cmax,
      ESIZ(t->children)
   );
   return t->children ? true : false;
}

extern void tree_prune(tree_t *t) {
   /* exit condition */
   if (t->clen == 0) {
      prune_callback(t);
      return;
   }

   for (size_t i = 0; i < t->clen; i++) {
      tree_prune(t->children[i]);
   }

   prune_callback(t);
}

static void prune_callback(tree_t *t) {
   arena_free_fallback(t->arena, t->children);
   arena_free_fallback(t->arena, t->dat);
   arena_free_fallback(t->arena, t);
}

extern void tree_destroy_shallow(tree_t *t) {
   arena_free_fallback(t->arena, t->children);
   /* t->dat not freed intentionally */
   arena_free_fallback(t->arena, t);
}

extern bool tree_pre_traverse(tree_t *t, tree_callback_t *cb, size_t lv, void *ctx) {
   if (!(*cb)(t, lv, ctx)) {
      return false;
   }
   if (t->clen == 0) {
      return true;
   }
   for (size_t i = 0; i < t->clen; i++) {
      if (!tree_pre_traverse(t->children[i], cb, lv + 1, ctx)) {
         return false;
      }
   }
   return true;
}

extern bool tree_post_traverse(tree_t *t, tree_callback_t *cb, size_t lv, void *ctx) {
   if (t->clen == 0) {
      return (*cb)(t, lv, ctx);
   }
   for (size_t i = 0; i < t->clen; i++) {
      if (!tree_post_traverse(t->children[i], cb, lv + 1, ctx)) {
         return false;
      }
   }
   return (*cb)(t, lv, ctx);
}

extern size_t tree_clen(const tree_t *t) {
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

extern void tree_setchild(const tree_t *t, int idx, tree_t *v) {
   t->children[idx] = v;
}

extern void tree_setdat(tree_t *t, void *d) {
   t->dat = d;
}

extern void tree_setparent(tree_t *t, tree_t *p) {
   t->parent = p;
}
