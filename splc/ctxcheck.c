#include "ctxcheck.h"
#include "ctxcheck.internals.h"

// TODO: conjugation이 제대로 됐는지같은걸 검사하도록 하자 (이건 warning을 띄워야 할듯)
// TODO: 미선언이름사용, 이름중복선언, scene/act 중복선언 같은것을 체크
// + I II III ... 넘버링이 순서대로 되는지

extern bool ctxcheck(compile_ctx_t *cctx) {
   ctxcheck_ctx_t octx = {
      .sourcefile = cctx->sourcefile,
      .cfg = cctx->cfg,
      .arena = cctx->arena,
      .ls = cctx->ls,
      .errcnt = 0,
      .errcnt_max = 50,
      .warncnt = 0,
      .warncnt_max = 50,
      .dp = tree_child(cctx->pt, 1),
      .nrtv = tree_child(cctx->pt, 2)
   };
   bool ret = true;

   if ((octx.errs = array_create(NULL, cctx->arena)) == NULL)
      goto OOM1;
   if ((octx.warns = array_create(NULL, cctx->arena)) == NULL)
      goto OOM2;

   if (cctx->cfg->vbs) {
      safe_fputs(stdout, ENPREFIX "context-checking...");
   }

   /* Warnings */
   if (check_unused_dp(&octx) != CtxcheckStatOk)
      goto report_warns;

report_warns:
   if (octx.warncnt > 0) {
      report_warnings(&octx);
   }

   /* Errors */
   if (check_invalid_goto(&octx) != CtxcheckStatOk)
      goto report_errors;
   if (check_empty_scene(&octx) != CtxcheckStatOk)
      goto report_errors;
   if (check_empty_line(&octx) != CtxcheckStatOk)
      goto report_errors;

report_errors:
   if (octx.errcnt > 0) {
      report_errors(&octx);
      ret = false;
   }

   if (ret && cctx->cfg->vbs) {
      safe_vprintf(" " Cgreen "done!" Creset "\n");
   }

   ;  /* octx.errs used arena */
OOM2:
   ;  /* octx.errs used arena */
OOM1:
   return ret;
}

static ctxcheck_statcode_t regerr(ctxcheck_ctx_t *octx) {
   if (!array_append(octx->errs, &octx->err, sizeof octx->err)) {
      return CtxcheckStatErr;
   }

   octx->errcnt++;

   return (octx->errcnt < octx->errcnt_max)
      ? CtxcheckStatOk
      : CtxcheckStatErrcntMax;
}

static ctxcheck_statcode_t regwarn(ctxcheck_ctx_t *octx) {
   /* Note: octx->err is intended */
   if (!array_append(octx->warns, &octx->warn, sizeof octx->warn)) {
      return CtxcheckStatErr;
   }

   octx->warncnt++;

   return (octx->warncnt < octx->warncnt_max)
      ? CtxcheckStatOk
      : CtxcheckStatWarncntMax;
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

static void report_warnings(ctxcheck_ctx_t *octx) {
   const size_t warns_cnt = array_size(octx->warns);
   ctxcheck_warn_ctx_t *warnctx;

   for (size_t i = 0; i < warns_cnt; i++) {
      warnctx = array_peek(octx->warns, i);
      (*warnctx->handle)(octx, warnctx, i);
   }

   if (octx->warncnt == octx->warncnt_max) {
      safe_vprintf("Warning count reached max. Warning check stopped.\n");
   }
}

static inline bool route_act(ctxcheck_ctx_t *octx) {
   octx->act = octx->t;
   return true;
}

static inline bool route_scene(ctxcheck_ctx_t *octx) {
   octx->scene = octx->t;
   return true;
}

static ctxcheck_statcode_t check_invalid_goto(ctxcheck_ctx_t *octx) {
   bool ret;

   octx->status = CtxcheckStatOk;
   ret = tree_pre_traverse(octx->nrtv, check_invalid_goto_route, 0, octx);

   return ret
      ? CtxcheckStatOk
      : CtxcheckStatErrcntMax;
}

static bool check_invalid_goto_route(tree_t *t, size_t lv, void *ctx) {
   ctxcheck_ctx_t *octx;
   bool ret;

   (void) lv;
   octx = ctx;
   octx->t = t;
   octx->n = tree_dat(octx->t);

   switch (octx->n->kind) {
      case NODEKIND_ACT:
         ret = route_act(octx);
      break;

      case NODEKIND_SCENE:
         ret = route_scene(octx);
      break;

      case NODEKIND_GOTO:
         ret = check_invalid_goto_target(octx);
      break;

      default: ret = true;
   }

   return ret;
}

static bool check_invalid_goto_target(ctxcheck_ctx_t *octx) {
   const nodekind_t dest = octx->n->dat.n;
   bool ret;

   switch (dest) {
      case NODEKIND_ACT:
         ret = check_invalid_goto_target_act(octx);
      break;

      case NODEKIND_SCENE:
         ret = check_invalid_goto_target_scene(octx);
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

static bool check_invalid_goto_target_act(ctxcheck_ctx_t *octx) {
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

   return (octx->status == CtxcheckStatOk)
      ? true
      : false;
}

static bool check_invalid_goto_target_scene(ctxcheck_ctx_t *octx) {
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

   return (octx->status == CtxcheckStatOk)
      ? true
      : false;
}

static ctxcheck_statcode_t check_empty_scene(ctxcheck_ctx_t *octx) {
   ctxcheck_statcode_t status = CtxcheckStatErr;
   size_t nact, nscn;
   tree_t *act, *scn;

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
         if ((status = regerr(octx)) != CtxcheckStatOk) {
            goto cleanup;
         }
      }
   }

   status = CtxcheckStatOk;

cleanup:
   return status;
}

static ctxcheck_statcode_t check_empty_line(ctxcheck_ctx_t *octx) {
   bool ret;

   octx->status = CtxcheckStatOk;
   ret = tree_pre_traverse(octx->nrtv, check_empty_line_route, 0, octx);

   return ret
      ? CtxcheckStatOk
      : CtxcheckStatErrcntMax;
}

static bool check_empty_line_route(tree_t *t, size_t lv, void *ctx) {
   ctxcheck_ctx_t *octx;
   bool ret;

   (void) lv;
   octx = ctx;
   octx->t = t;
   octx->n = tree_dat(octx->t);

   switch (octx->n->kind) {
      case NODEKIND_ACT:
         ret = route_act(octx);
      break;

      case NODEKIND_SCENE:
         ret = route_scene(octx);
      break;

      case NODEKIND_LINE:
         ret = check_empty_line_target(octx);
      break;

      default: ret = true;
   }

   return ret;
}

static bool check_empty_line_target(ctxcheck_ctx_t *octx) {
   /* line
         -> char
         -> stmt... */

   if (tree_clen(octx->t) > 1) {
      return true;
   }

   /* error path */
   octx->err = (ctxcheck_err_ctx_t) {
      .act = octx->act,
      .scene = octx->scene,
      .curr = octx->t,
      .reason = "this line is empty",
      .handle = ctxcheck_errhandle_default
   };
   octx->status = regerr(octx);

   return (octx->status == CtxcheckStatOk)
      ? true
      : false;
}

static ctxcheck_statcode_t check_unused_dp(ctxcheck_ctx_t *octx) {
   size_t dp_clen;
   ctxcheck_statcode_t status;

   dp_clen = tree_clen(octx->dp);
   octx->dp_counters = arena_calloc(
      octx->arena,
      dp_clen,
      sizeof *octx->dp_counters
   );
   if (!octx->dp_counters) {
      return CtxcheckStatErr;
   }

   tree_pre_traverse(octx->nrtv, check_unused_dp_route, 0, octx);

   for (size_t i = 0; i < dp_clen; i++) {
      if (octx->dp_counters[i] > 0) {
         continue;
      }
      /* error path */
      octx->warn = (ctxcheck_warn_ctx_t) {
         .act = NULL,
         .scene = NULL,
         .curr = tree_child(octx->dp, i),
         /* fixme maybe this hack? support ctx member instead */
         .reason = (char *) i,  /* to be used as charidx */
         .handle = ctxcheck_warnhandle_unused_dp
      };
      status = regwarn(octx);
      if (status == CtxcheckStatWarncntMax) {
         goto cleanup;
      }
   }

   status = CtxcheckStatOk;

cleanup:
   arena_free_fallback(octx->arena, octx->dp_counters);

   return status;
}

static bool check_unused_dp_route(tree_t *t, size_t lv, void *ctx) {
   ctxcheck_ctx_t *octx;
   size_t charidx;

   (void) lv;
   octx = ctx;
   octx->t = t;
   octx->n = tree_dat(octx->t);

   if (octx->n->kind != NODEKIND_CHAR) {
      return true;
   }

   charidx = octx->n->dat.n;
   octx->dp_counters[charidx]++;

   return true;
}

static void ctxcheck_errhandle_default(
   ctxcheck_ctx_t *superctx,
   ctxcheck_err_ctx_t *errctx,
   size_t idx
) {
   serr_header(idx);
   safe_vprintf("%s\n", errctx->reason);
   diag_template("at here", superctx, errctx->curr);
   under_scene(superctx, errctx->scene);
   under_act(superctx, errctx->act);
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

static void ctxcheck_warnhandle_unused_dp(
   ctxcheck_ctx_t *superctx,
   ctxcheck_err_ctx_t *errctx,
   size_t idx
) {
   node_t *chardecl_node = tree_dat(errctx->curr);
   size_t charidx = (size_t) errctx->reason;

   swarn_header(idx);
   /* assumes that name concatenation done already */
   safe_vprintf(
      Cemerald "%s" Creset " was declared as "
      Cyellow "dp[%zu]" Creset
      " but not used throughout the source file\n",
      chardecl_node->dat.s.run, charidx
   );
   diag_template("declared at here", superctx, errctx->curr);
}

static inline void serr_header(size_t idx) {
   safe_vprintf(
      "\n"
      Cbred "> Semantic Error #%zu: " Creset,
      idx + 1
   );
}

static inline void swarn_header(size_t idx) {
   safe_vprintf(
      "\n"
      Cyellow "> Semantic Warning #%zu: " Creset,
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
      "%4d|%s",
      ctx->sourcefile, di.dat->lnum, di.dat->lpos,
      kind, rn->dat.s.run,
      di.dat->lnum, di.ln->run
   );
}

static void diag_template(const char *msg, ctxcheck_ctx_t *ctx, tree_t *t) {
   diag_info_t di = get_di(ctx, t);
   safe_vprintf(
      "[%s:%zu:%zu] " Cbwhite "note: " Creset "%s\n"
      "%4d|" Cbmagenta "%s" Creset,
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
