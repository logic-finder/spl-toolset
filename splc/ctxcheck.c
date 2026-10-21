#include "ctxcheck.h"
#include "ctxcheck.internals.h"

// TODO: conjugation이 제대로 됐는지같은걸 검사하도록 하자 (이건 warning을 띄워야 할듯)
// TODO: 미선언이름사용, 이름중복선언, 미사용이름체크, scene/act 중복선언 같은것을 체크
// + I II III ... 넘버링이 순서대로 되는지

extern void ctxcheck(compile_ctx_t *cctx) {
   ctxcheck_ctx_t octx = {0};

   octx.sourcefile = cctx->sourcefile;
   octx.cfg = cctx->cfg;
   octx.ls = cctx->ls;
   octx.errs = array_create(NULL);
   octx.errcnt = 0;
   octx.errcnt_max = 50;
   octx.nrtv = tree_child(cctx->pt, 2);

   if (cctx->cfg->vbs) safe_fputs(stdout, ENPREFIX "context-checking...");

   check_goto_unexistent(&octx);
   if (check_empty_scene(&octx) != CtxcheckStatcodeOk) {
      goto report_errors;
   }

report_errors:
   if (octx.errcnt > 0) {
      report_errors(&octx);
      goto cleanup;
   }

   if (cctx->cfg->vbs) safe_vprintf(" " Cgreen "done!" Creset "\n");

cleanup:
   return;
}

static ctxcheck_statcode_t regerr(ctxcheck_ctx_t *octx) {
   array_append(octx->errs, &octx->err, sizeof octx->err);

   octx->errcnt++;

   return (octx->errcnt < octx->errcnt_max)
      ? CtxcheckStatcodeOk
      : CtxcheckStatcodeErrcntMax;
}

static void report_errors(ctxcheck_ctx_t *octx) {
   const size_t errs_cnt = array_size(octx->errs);
   ctxcheck_err_ctx_t *errctx;

   for (size_t i = 0; i < errs_cnt; i++) {
      errctx = array_peek(octx->errs, i);
      (*errctx->handle)(octx, errctx, i);
   }

   if (octx->errcnt == octx->errcnt_max) {
      safe_vprintf("Error count reached max. Aborting.\n");
   }
}

static inline void check_goto_unexistent(ctxcheck_ctx_t *octx) {
   tree_pre_traverse(octx->nrtv, check_goto_unexistent_route, 0, octx);
}

static void check_goto_unexistent_route(tree_t *t, int lv, void *ctx) {
   ctxcheck_ctx_t *octx;

   (void) lv;

   octx = ctx;
   octx->t = t;
   octx->n = tree_dat(octx->t);

   switch(octx->n->kind) {
      case NODEKIND_ACT   : check_goto_unexistent_act  (octx); break;
      case NODEKIND_SCENE : check_goto_unexistent_scene(octx); break;
      case NODEKIND_GOTO  : check_goto_unexistent_goto (octx); break;
      default: ;
   }
}

static inline void check_goto_unexistent_act(ctxcheck_ctx_t *octx) {
   octx->act = octx->t;
}

static inline void check_goto_unexistent_scene(ctxcheck_ctx_t *octx) {
   octx->scene = octx->t;
}

static void check_goto_unexistent_goto(ctxcheck_ctx_t *octx) {
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

static ctxcheck_statcode_t check_empty_scene(ctxcheck_ctx_t *octx) {
   size_t nact, nscn;
   tree_t *act, *scn;
   ctxcheck_statcode_t status;

   /* nrtv
      -> act
         -> romnum (first child)
         -> scene
            -> romnum (first child) */

   nact = tree_clen(octx->nrtv);

   for (size_t a = 0; a < nact; a++) {
      act = tree_child(octx->nrtv, a);
      nscn = tree_clen(act);

      for (size_t s = 1; s < nscn; s++) {
         scn = tree_child(act, s);

         if (tree_clen(scn) != 1) {
            continue;
         }

         /* error path */
         octx->err = (ctxcheck_err_ctx_t) {
            .act = act,
            .scene = scn,
            .handle = ctxcheck_errhandle_empty_scene
         };
         status = regerr(octx);
         if (status == CtxcheckStatcodeErrcntMax) {
            goto cleanup;
         }
      }
   }

cleanup:
   return status;
}

static void ctxcheck_errhandle_empty_scene(
   ctxcheck_ctx_t *superctx,
   ctxcheck_err_ctx_t *errctx,
   size_t idx
) {
   node_t *act_dat = tree_dat(errctx->act);
   line_t *act_ln = array_peek(superctx->ls, act_dat->lnum - 1);
   node_t *act_romnum = tree_chdat(errctx->act, 0);

   node_t *scene_dat = tree_dat(errctx->scene);
   line_t *scene_ln = array_peek(superctx->ls, scene_dat->lnum - 1);
   node_t *scene_romnum = tree_chdat(errctx->scene, 0);

   safe_vprintf("\n");
   safe_vprintf(
      Cbred "> Semantic Error #%zu: " Creset
      "the " Cul "Scene %s" Creset " is empty\n",
      idx + 1, scene_romnum->dat.s.run
   );
   safe_vprintf(
      "[%s:%zu:%zu] " Cbwhite "note:" Creset " defined at here\n"
      "%4d|" Cbmagenta "%s\n" Creset,
      superctx->sourcefile, scene_dat->lnum, scene_dat->lpos,
      scene_dat->lnum, scene_ln->run
   );
   safe_vprintf(
      "[%s:%zu:%zu] " Cbwhite "note:" Creset " under the "
      Cul "Act %s\n" Creset
      "%4d|%s\n",
      superctx->sourcefile, act_dat->lnum, act_dat->lpos,
      act_romnum->dat.s.run,
      act_dat->lnum, act_ln->run
   );
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
      octx->sourcefile, lnum, lpos,
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
      octx->sourcefile, lnum, lpos,
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
   // TODO: do not halt on the first error
   exit(EXIT_FAILURE);
}
