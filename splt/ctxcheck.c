#include "ctxcheck.h"
#include "ctxcheck.internals.h"

extern void ctxcheck(optflg_t *of, optval_t *ov) {
   nrtv = tree_child(pt, 2);
   tree_pre_traverse(nrtv, ctxcheck_router, 0);
}

static void ctxcheck_router(tree_t *t, int _) {
   node_t *n;

   (void) _;
   n = tree_dat(t);

   switch(n->kind) {
      case NODEKIND_ACT   : ctxcheck_act  (t); break;
      case NODEKIND_SCENE : ctxcheck_scene(t); break;
      case NODEKIND_GOTO  : ctxcheck_goto (t); break;
      default : ;
   }
}

static void ctxcheck_act(tree_t *t) {
   act = t;
}

static void ctxcheck_scene(tree_t *t) {
   scene = t;
}

static void ctxcheck_goto(tree_t *t) {
   const tree_t *root, *root_child;
   const char *romnum, *against;
   node_t *n;
   int mark, root_len;

   n = tree_dat(t);
   mark = n->dat.n;
   romnum = TREE_CHDAT(t, 0)->dat.s.run;

   if (mark == NODEKIND_ACT)
      root = nrtv;  /* root_child is NODEKIND_ACT   */
   else  /* Scene */
      root = act;   /* root_child is NODEKIND_SCENE */

   root_len = tree_clen(root);
   for (int i = 1; i < root_len; i++) {  /* [0] = NODEKIND_ROMNUM */
      root_child = tree_child(root, i);
      against = TREE_CHDAT(root_child, 0)->dat.s.run;
      if (!strcmp(romnum, against))
         return;  /* found */
   }

   if (root == nrtv)
      reason = msgs.err.sem.no_such_act;
   else
      reason = msgs.err.sem.no_such_scene;
   semerr_unknown_label(n, romnum);
}

static void print_err(node_t *n) {
   int lnum, lpos;
   line_t *l;

   lnum = n->lnum;
   lpos = n->lpos;
   l = arr_peek(ls, lnum - 1);

   fmtwrt(
      "\n[%s:%d:%d] " Cbwhite "note:" Creset " at this goto statement"
      "\n%4d|%s",
      sfname, lnum, lpos,
      lnum, l->run
   );
}

static void trace(tree_t *t, const char *type) {
   int lnum, lpos;
   line_t *l;
   node_t *rootnode, *childnode;

   rootnode = tree_dat(t);
   lnum = rootnode->lnum;
   lpos = rootnode->lpos;
   l = arr_peek(ls, lnum - 1);
   childnode = tree_chdat(t, 0);

   fmtwrt(
      "\n[%s:%d:%d] " Cbwhite "note:" Creset
      " in the " Cbmagenta "%s %s" Creset
      "\n%4d|%s",
      sfname, lnum, lpos,
      type, childnode->dat.s.run,
      lnum, l->run
   );
}

static void semerr_unknown_label(node_t *n, const char *s) {
   sfputs(stdout, Cbred "\n<semantic error> " Creset);
   fmtwrt(reason, s);
   print_err(n);
   trace(scene, "Scene");
   trace(act, "Act");
   sfputc(stdout, '\n');
   exit(EXIT_FAILURE);
}
