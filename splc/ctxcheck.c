#include "ctxcheck.h"
#include "ctxcheck.internals.h"

// TODO: conjugation이 제대로 됐는지같은걸 검사하도록 하자 (이건 warning을 띄워야 할듯)
// TODO: 미선언이름사용, 이름중복선언, 미사용이름체크, scene/act 중복선언 같은것을 체크

extern void ctxcheck(compile_ctx_t *cctx) {
   ctxcheck_ctx_t octx;

   octx.ls = cctx->ls;
   octx.ov = cctx->ov;
   octx.nrtv = tree_child(cctx->pt, 2);

   if (cctx->of->vbs) safe_fputs(stdout, ENPREFIX "context-checking...");

   tree_pre_traverse(octx.nrtv, ctxcheck_router, 0, &octx);

   if (cctx->of->vbs) safe_vprintf(" " Cgreen "done!" Creset "\n");
}

static void ctxcheck_router(tree_t *t, int lv, void *ctx) {
   ctxcheck_ctx_t *octx;

   (void) lv;

   octx = ctx;
   octx->t = t;
   octx->n = tree_dat(octx->t);

   switch(octx->n->kind) {
      case NODEKIND_ACT   : ctxcheck_act  (octx); break;
      case NODEKIND_SCENE : ctxcheck_scene(octx); break;
      case NODEKIND_GOTO  : ctxcheck_goto (octx); break;
      default: ;
   }
}

static inline void ctxcheck_act(ctxcheck_ctx_t *octx) {
   octx->act = octx->t;
}

static inline void ctxcheck_scene(ctxcheck_ctx_t *octx) {
   octx->scene = octx->t;
}

static void ctxcheck_goto(ctxcheck_ctx_t *octx) {
   const tree_t *root, *root_child;
   const char *romnum, *against;
   size_t i, root_len;
   nodekind_t mark;
   node_t *chdat;

   mark = octx->n->dat.n;
   chdat = tree_chdat(octx->t, 0);
   romnum = chdat->dat.s.run;

   if (mark == NODEKIND_ACT) {
      i = 0;
      root = octx->nrtv;  /* root_child = NODEKIND_ACT */
   }
   else {  /* Scene */
      i = 1;
      root = octx->act;   /* root_child = NODEKIND_SCENE */
   }

   root_len = tree_clen(root);
   for ( ; i < root_len; i++) {
      root_child = tree_child(root, i);
      against = TREE_CHDAT(root_child, 0)->dat.s.run;
      if (!strcmp(romnum, against)) {
         return;  /* found */
      }
   }

   if (root == octx->nrtv) {
      octx->reason = msgs.err.sem.no_such_act;
   }
   else {
      octx->reason = msgs.err.sem.no_such_scene;
   }
   semerr_unknown_label(octx, romnum);
}

static void print_err(ctxcheck_ctx_t *octx) {
   size_t lnum, lpos;
   line_t *l;

   lnum = octx->n->lnum;
   lpos = octx->n->lpos;
   l = array_peek(octx->ls, lnum - 1);

   safe_vprintf(
      "\n[%s:%zu:%zu] " Cbwhite "note:" Creset " at this goto statement"
      "\n%4d|%s",
      octx->ov->src, lnum, lpos,
      lnum, l->run
   );
}

static void trace(ctxcheck_ctx_t *octx, tree_t *t, const char *type) {
   size_t lnum, lpos;
   line_t *l;
   node_t *rootnode, *childnode;

   rootnode = tree_dat(t);
   lnum = rootnode->lnum;
   lpos = rootnode->lpos;
   l = array_peek(octx->ls, lnum - 1);
   childnode = tree_chdat(t, 0);

   safe_vprintf(
      "\n[%s:%zu:%zu] " Cbwhite "note:" Creset
      " in the " Cbmagenta "%s %s" Creset
      "\n%4d|%s",
      octx->ov->src, lnum, lpos,
      type, childnode->dat.s.run,
      lnum, l->run
   );
}

static void semerr_unknown_label(ctxcheck_ctx_t *octx, const char *s) {
   safe_fputs(stdout, Cbred "\n<semantic error> " Creset);
   safe_vprintf(octx->reason, s);
   print_err(octx);
   trace(octx, octx->scene, "Scene");
   trace(octx, octx->act, "Act");
   safe_fputc(stdout, '\n');
   exit(EXIT_FAILURE);
}
