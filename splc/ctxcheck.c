#include "ctxcheck.h"
#include "ctxcheck.internals.h"

// TODO: conjugation이 제대로 됐는지같은걸 검사하도록 하자 (이건 warning을 띄워야 할듯)
// TODO: 미선언이름사용, 이름중복선언, 미사용이름체크, scene/act 중복선언 같은것을 체크
// + I II III ... 넘버링이 순서대로 되는지

extern void ctxcheck(compile_ctx_t *cctx) {
   ctxcheck_ctx_t octx = {
      .sourcefile = cctx->sourcefile,
      .cfg = cctx->cfg,
      .ls = cctx->ls,
      .errs = array_create(NULL),
      .errcnt = 0,
      .errcnt_max = 50,
      .nrtv = tree_child(cctx->pt, 2)
   };

   if (cctx->cfg->vbs) safe_fputs(stdout, ENPREFIX "context-checking...");

   if (check_invalid_goto(&octx) != CtxcheckStatcodeOk)
      goto report_errors;
   if (check_empty_scene(&octx) != CtxcheckStatcodeOk)
      goto report_errors;

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

static ctxcheck_statcode_t check_invalid_goto(ctxcheck_ctx_t *octx) {
   bool ret;

   octx->status = CtxcheckStatcodeOk;
   ret = tree_pre_traverse(octx->nrtv, check_invalid_goto_route, 0, octx);

   return ret
      ? CtxcheckStatcodeOk
      : CtxcheckStatcodeErrcntMax;
}

static bool check_invalid_goto_route(tree_t *t, size_t lv, void *ctx) {
   ctxcheck_ctx_t *octx;
   bool ret;

   (void) lv;

   octx = ctx;
   octx->t = t;
   octx->n = tree_dat(octx->t);

   switch(octx->n->kind) {
      case NODEKIND_ACT:
         ret = check_invalid_goto_act(octx);
      break;

      case NODEKIND_SCENE:
         ret = check_invalid_goto_scene(octx);
      break;

      case NODEKIND_GOTO:
         ret = check_invalid_goto_goto (octx);
      break;

      default: ret = true;
   }

   return ret;
}

static inline bool check_invalid_goto_act(ctxcheck_ctx_t *octx) {
   octx->act = octx->t;
   return true;
}

static inline bool check_invalid_goto_scene(ctxcheck_ctx_t *octx) {
   octx->scene = octx->t;
   return true;
}

static bool check_invalid_goto_goto(ctxcheck_ctx_t *octx) {
   const nodekind_t dest = octx->n->dat.n;
   bool ret;

   switch (dest) {
      case NODEKIND_ACT:
         ret = check_invalid_goto_goto_act(octx);
      break;

      case NODEKIND_SCENE:
         ret = check_invalid_goto_goto_scene(octx);
      break;

      /* control never reaches default because `goto` node
         has either NODEKIND_ACT or NODEKIND_SCENE because
         of parse_goto()'s logic */
      default: ret = false;
      /* Note: return false means that the tree traversal
         needs to stop */
   }

   return ret;
}

static bool check_invalid_goto_goto_act(ctxcheck_ctx_t *octx) {
   const size_t num_acts = tree_clen(octx->nrtv);
   const tree_t *act;
   const node_t *goto_rn_dat, *act_rn_dat;
   const char *goto_rn_str, *act_rn_str;

   /* nrtv
         -> act
            -> romnum (first child)
      goto
         -> romnum (first child) */

   goto_rn_dat = tree_chdat(octx->t, 0);
   goto_rn_str = goto_rn_dat->dat.s.run;

   for (size_t i = 0; i < num_acts; i++) {
      act = tree_child(octx->nrtv, i);
      act_rn_dat = tree_chdat(act, 0);
      act_rn_str = act_rn_dat->dat.s.run;

      if (!strcmp(goto_rn_str, act_rn_str)) {
         return true;
         /* Note: return true means that the tree traversal
            can continue */
      }
   }

   /* error path */
   octx->err = (ctxcheck_err_ctx_t) {
      .act = octx->act,
      .scene = octx->scene,
      .curr = octx->t,
      .reason = NULL,
      .handle = ctxcheck_errhandle_invalid_goto
   };
   octx->status = regerr(octx);
   if (octx->status == CtxcheckStatcodeErrcntMax) {
      goto cleanup;
   }

cleanup:
   return (octx->status == CtxcheckStatcodeOk)
      ? true
      : false;
}

static bool check_invalid_goto_goto_scene(ctxcheck_ctx_t *octx) {
   const size_t act_clen = tree_clen(octx->act);
   const tree_t *scene;
   const node_t *goto_rn_dat, *scene_rn_dat;
   const char *goto_rn_str, *scene_rn_str;

   /* nrtv
         -> act...
            -> romnum (first child)
            -> scene...
               -> romnum (first child)
      goto
         -> romnum (first child) */

   goto_rn_dat = tree_chdat(octx->t, 0);
   goto_rn_str = goto_rn_dat->dat.s.run;

   /* Note that `i` begins at 1 because the first child
      of act is not scene but the act's roman numeral node */
   for (size_t i = 1; i < act_clen; i++) {
      scene = tree_child(octx->act, i);
      scene_rn_dat = tree_chdat(scene, 0);
      scene_rn_str = scene_rn_dat->dat.s.run;

      if (!strcmp(goto_rn_str, scene_rn_str)) {
         return true;
         /* Note: return true means that the tree traversal
            can continue */
      }
   }

   /* error path */
   octx->err = (ctxcheck_err_ctx_t) {
      .act = octx->act,
      .scene = octx->scene,
      .curr = octx->t,
      .reason = NULL,
      .handle = ctxcheck_errhandle_invalid_goto
   };
   octx->status = regerr(octx);
   if (octx->status == CtxcheckStatcodeErrcntMax) {
      goto cleanup;
   }

cleanup:
   return (octx->status == CtxcheckStatcodeOk)
      ? true
      : false;
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
            .curr = scn,
            .reason = NULL,
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

static void ctxcheck_errhandle_invalid_goto(
   ctxcheck_ctx_t *superctx,
   ctxcheck_err_ctx_t *errctx,
   size_t idx
) {
   node_t *goto_dat = tree_dat(errctx->curr);
   node_t *goto_romnum = tree_chdat(errctx->curr, 0);

   serr_header(idx);
   safe_vprintf(
      "jump to the nonexistent " Cbmagenta "%s %s\n" Creset,
      (goto_dat->dat.n == NODEKIND_ACT) ? "Act" : "Scene",
      goto_romnum->dat.s.run
   );
   diag_template("at here", superctx, errctx->curr);
   under_scene(superctx, errctx->scene);
   under_act(superctx, errctx->act);
}

static void ctxcheck_errhandle_empty_scene(
   ctxcheck_ctx_t *superctx,
   ctxcheck_err_ctx_t *errctx,
   size_t idx
) {
   node_t *rn = tree_chdat(errctx->scene, 0);

   serr_header(idx);
   safe_vprintf(
      "the " Cbmagenta "Scene %s" Creset " is empty\n",
      rn->dat.s.run
   );
   diag_template("defined at here", superctx, errctx->scene);
   under_act(superctx, errctx->act);
}

static inline void serr_header(size_t idx) {
   safe_vprintf(
      "\n"
      Cbred "> Semantic Error #%zu: " Creset,
      idx + 1
   );
}

static inline void under_act(ctxcheck_ctx_t *ctx, tree_t *act) {
   under_template("Act", ctx, act);
}

static inline void under_scene(ctxcheck_ctx_t *ctx, tree_t *scene) {
   under_template("Scene", ctx, scene);
}

static void under_template(const char *kind, ctxcheck_ctx_t *ctx, tree_t *t) {
   diag_info_t di;
   node_t *rn;

   di = get_di(ctx, t);
   rn = tree_chdat(t, 0);

   safe_vprintf(
      "[%s:%zu:%zu] " Cbwhite "note:" Creset " under the "
      Cemerald "%s %s\n" Creset
      "%4d|%s\n",
      ctx->sourcefile, di.dat->lnum, di.dat->lpos,
      kind, rn->dat.s.run,
      di.dat->lnum, di.ln->run
   );
}

static void diag_template(const char *msg, ctxcheck_ctx_t *ctx, tree_t *t) {
   diag_info_t di = get_di(ctx, t);
   safe_vprintf(
      "[%s:%zu:%zu] " Cbwhite "note: " Creset "%s\n"
      "%4d|%s\n",
      ctx->sourcefile, di.dat->lnum, di.dat->lpos, msg,
      di.dat->lnum, di.ln->run
   );
}

static diag_info_t get_di(ctxcheck_ctx_t *ctx, tree_t *t) {
   diag_info_t ret = {0};

   ret.dat = tree_dat(t);
   ret.ln = array_peek(ctx->ls, ret.dat->lnum - 1);

   return ret;
}
