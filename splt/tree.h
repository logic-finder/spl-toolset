#ifndef TREE_H
#define TREE_H

#include <stddef.h>
#include <string.h>

#define TAGLEN 31

typedef struct tree tree_t;

typedef struct tree {
   tree_t *parent;
   tree_t **child;
   int cmax;
   int clen;
   char *data;
   size_t dsiz;
   char tag[TAGLEN + 1];
} tree_t;

tree_t *tree_plant(void *data, size_t dsiz);
void tree_addchild(tree_t *base, tree_t *incoming);
void tree_prune(tree_t *root);
tree_t *tree_graft(
   tree_t *base,
   void *data,
   size_t dsiz,
   const char *tag
);
// tree_t *tree_sgraft(
//    tree_t *base,
//    const char *data,
//    const char *tag
// );

inline tree_t *tree_sgraft(
   tree_t *base,
   const char *data,
   const char *tag
) {
   return tree_graft(
      base,
      (void *) data,
      strlen(data) + 1,
      tag
   );
}

#endif
