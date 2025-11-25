#include <stddef.h>
#include <string.h>
#include "fatal.h"
#include "wrapper.h"
#include "global.h"
#include "tree.h"

#define CMAX_INIT 2

extern tree_t *tree_plant(void *data, size_t dsiz) {
   tree_t *ret;  /* my little sapling! */

   ret = smalloc(sizeof *ret);
   ret->child = smalloc(sizeof (tree_t *) * CMAX_INIT);
   if (data)
      ret->data = smalloc(dsiz);
   else
      ret->data = NULL;

   ret->parent = NULL;
   ret->cmax = CMAX_INIT;
   ret->clen = 0;
   ret->dsiz = dsiz;
   memcpy(ret->data, data, dsiz);
   strcpy(ret->tag, "init");

   return ret;
}

extern void tree_addchild(tree_t *base, tree_t *incoming) {
   if (incoming->parent)
      VERR("%s: arg2 has a parent node.", __func__);

   if (base->cmax == base->clen) {
      base->cmax *= 2;
      base->child = srealloc(
         base->child, sizeof (tree_t *) * base->cmax);
   }

   incoming->parent = base;
   base->child[base->clen++] = incoming;
}

extern void tree_prune(tree_t *root) {
   if (root->parent)
      VERR("%s: arg1 is not a root node.", __func__);

   if (root->clen == 0) {
      free(root->child);
      free(root->data);
      free(root);
      return;
   }
   for (int i = 0; i < root->clen; i++)
      tree_prune(root->child[i]);
}
