#ifndef TREE_H
#define TREE_H

#include <stddef.h>

typedef struct tree tree_t;

typedef struct tree {
   tree_t *parent;
   tree_t **child;
   int cmax;
   int clen;
   void *data;
   size_t dsiz;
   char tag[32];
} tree_t;

tree_t *tree_plant(void *data, size_t dsiz);
void tree_addchild(tree_t *base, tree_t *incoming);
void tree_prune(tree_t *root);

#endif
