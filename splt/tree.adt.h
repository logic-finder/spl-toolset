#ifndef TREE_ADT_H
#define TREE_ADT_H

#include <stddef.h>
#include <string.h>

/************
 * TYPEDEFS *
 ************/
typedef struct tree tree_t;
typedef void tree_callback_t(tree_t *tree, int lv, void *ctx);
typedef void tree_walker_t(tree_t *t, tree_callback_t *cb, int lv, void *ctx);

/***********************
 * FUNCTION PROTOTYPES *
 ***********************/
tree_t *tree_plant(void *data, size_t dsiz);
tree_t *tree_graft(
   tree_t * restrict base,
   tree_t * restrict in
);
void tree_prune(tree_t *t);
tree_walker_t tree_post_traverse;
tree_walker_t tree_pre_traverse;
int tree_clen(const tree_t *t);
void *tree_dat(const tree_t *t);
tree_t *tree_child(const tree_t *t, int idx);
void *tree_chdat(const tree_t *t, int idx);
tree_t *tree_parent(const tree_t *t);

#endif
