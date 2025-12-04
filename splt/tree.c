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
   ret->child = smalloc(CMAX_INIT * sizeof *ret->child);
   if (data)
      ret->data = smalloc(dsiz);
   else
      ret->data = NULL;

   ret->parent = NULL;
   ret->cmax = CMAX_INIT;
   ret->clen = 0;
   ret->dsiz = dsiz;
   memcpy(ret->data, data, dsiz);
   strcpy(ret->tag, "(empty)");

   return ret;
}

extern void tree_addchild(tree_t *base, tree_t *incoming) {
   if (incoming->parent)
      VERR("%s: arg2 has a parent node.", __func__);

   if (base->cmax == base->clen) {
      base->cmax *= 2;
      base->child = srealloc(
         base->child, base->cmax * sizeof *base->child);
   }

   incoming->parent = base;
   base->child[base->clen++] = incoming;
}

extern void tree_prune(tree_t *root) {
   // if (root->parent)
   //    VERR("%s: arg1 is not a root node.", __func__);

   if (root->clen == 0) {
      free(root->child);
      free(root->data);
      free(root);
      return;
   }
   for (int i = 0; i < root->clen; i++)
      tree_prune(root->child[i]);
}

extern tree_t *tree_graft(
   tree_t *base,
   void *data,
   size_t dsiz,
   const char *tag
) {
   tree_t *sapling;

   sapling = tree_plant(data, dsiz);
   strncpy(sapling->tag, tag, TAGLEN + 1);
   tree_addchild(base, sapling);

   return sapling;
}

// extern tree_t *tree_sgraft(
//    tree_t *base,
//    const char *data,
//    const char *tag
// ) {
//    return tree_graft(
//       base,
//       (void *) data,
//       strlen(data) + 1,
//       tag
//    );
// }

extern tree_t *tree_sgraft(
   tree_t *base,
   const char *data,
   const char *tag
);
