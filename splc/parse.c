#include "parse.h"
#include "parse.internals.h"

// fixme: STREQL 매크로 정의해서 쓰기 (!strcmp 대체 프로젝트 전체적으로)

extern void parse(compile_ctx_t *cctx) {
   parse_ctx_t pctx;

   pctx.of = cctx->of;
   pctx.ov = cctx->ov;
   pctx.ls = cctx->ls;
   pctx.toks = cctx->toks;
   pctx.len = array_size(pctx.toks);
   pctx.idx = 0;
   pctx.tok = array_peek(pctx.toks, pctx.idx);
   pctx.pt = plant_tree(NULL, 0, NODEKIND_ROOT, 0, 0);
   pctx.errcnt = 0;
   pctx.errs = array_create(NULL);

   if (cctx->of->vbs) safe_fputs(stdout, ENPREFIX "parsing...");

   /* Constructs the parse tree
         pt[0] = title
         pt[1] = dp
         pt[2] = nrtv  */

   parse_title(&pctx);
   parse_dp(&pctx);
   pctx.nrtv = graft_tree_n(pctx.pt, 0, NODEKIND_NRTV, pctx.tok);
   for (;;) {
      parse_act(&pctx);
      for (;;) {
         parse_scene(&pctx);
         switch (parse_stmt(&pctx)) {
            case NODEKIND_SCENE   : goto EOS;
            case NODEKIND_ACT     : goto EOA;
            case NODEKIND__FINALE : goto EOE;
         }
      EOS:;
      }
   EOA:;
   }
EOE:;

   /* Reports collected syntax errors, if any */
   if (pctx.errcnt > 0) {
      report_syntax_errors(&pctx);
      exit(EXIT_FAILURE);
   }

   if (cctx->of->vbs) safe_vprintf(" " Cgreen "done!" Creset
      "\t(total " Cbwhite "%zu" Creset " nodes)\n",
      count_tree_node(pctx.pt)
   );

   cctx->pt = pctx.pt;
   return;
}

extern void destroy_pt(compile_ctx_t *cctx) {
   tree_post_traverse(cctx->pt, cleanup_node, 0, NULL);
   tree_prune(cctx->pt);
}

static void cleanup_node(tree_t *t, int lv, void *ctx) {
   node_t *n;

   (void) lv, (void) ctx;

   n = tree_dat(t);

   if (n->datkind != DATKIND_STR) {
      return;
   }

   free(n->dat.s.run);
}

static void report_syntax_errors(parse_ctx_t *pctx) {
   array_foreach(pctx->errs, print_syntax_error, pctx);

   if (pctx->errcnt < pctx->errcnt_max) {
      return;
   }

   safe_vprintf("too many syntax errors."
      " Aborting parsing at [%s:%zu:%zu]\n",
      pctx->ov->src, pctx->etok->lnum, pctx->etok->lpos
   );
}

static void parse_title(parse_ctx_t *pctx) {
   tree_t *title;

   title = graft_tree_n(pctx->pt, 0, NODEKIND_TITLE, pctx->tok);

   graft_tree_s(title, NODEKIND_DATA, pctx->tok);

   pctx->reason = msgs.err.syn.title.incomp;
   readtoks_until(pctx, ".!?", title);
}

static void parse_dp(parse_ctx_t *pctx) {
   tree_t *dp, *character;

   dp = graft_tree_n(pctx->pt, 0, NODEKIND_DP, pctx->tok);

   for (;;) {
      character = graft_tree_n(dp, 0, NODEKIND_CHDECL, pctx->tok);

      pctx->reason = msgs.err.syn.dp.chardecl_incomp;
      readtoks(pctx, ',', character);

      if (tree_clen(character) == 0) {
         pctx->reason = msgs.err.syn.dp.noname;
         regerr(pctx);
      }

      pctx->reason = msgs.err.syn.dp.desc_incomp;
      skiptoks(pctx, ".!?");

      pctx->reason = msgs.err.syn.dp.nonext;
      gettok(pctx);
      if (seek_act(pctx)) {
         break;
      }
      ungettok(pctx);

      // fixme: here
      // coalesce_name();
      // if (!strcmp(pctx->ov->std, stdopt_cor27)) {
      //    continue;
      // }
      // if (!is_shakespearean_name()) {
      //    pctx->reason = "not an shakespearean name";
      //    regerr(pctx);
      // }
   }
}

static int seek_act(parse_ctx_t *pctx) {
   return strcasecmp(pctx->tok->run, KEYWRD_ACT) ? 0 : 1;
}

static void parse_act(parse_ctx_t *pctx) {
   if (pctx->of->w_kc && strcmp(pctx->tok->run, KEYWRD_ACT)) {
      pctx->reason = "this keyword should be 'Act'";
      synwarn(pctx);
   }

   pctx->act = graft_tree_n(
      pctx->nrtv, 0, NODEKIND_ACT, pctx->tok
   );

   pctx->reason = msgs.err.syn.act.incomp;
   gettok(pctx);

   if (!is_rnum(pctx->tok->run)) {
      graft_tree_s(pctx->act, NODEKIND__ERROR, pctx->tok);
      pctx->reason = msgs.err.syn.act.nornum;
      regerr(pctx);
      resync(pctx, ":");
   }
   else {
      graft_tree_s(pctx->act, NODEKIND_ROMNUM, pctx->tok);
      gettok(pctx);
   }

   if (pctx->tok->run[0] != ':') {
      pctx->reason = msgs.err.syn.act.nocolon;
      regerr(pctx);
   }

   pctx->reason = msgs.err.syn.act.desc_incomp;
   skiptoks(pctx, ".!?");

   pctx->reason = msgs.err.syn.act.noscene;
   gettok(pctx);

   if (!seek_scene(pctx)) synerr(pctx);
}

static int seek_scene(parse_ctx_t *pctx) {
   return strcasecmp(pctx->tok->run, KEYWRD_SCENE) ? 0 : 1;
}

static void parse_scene(parse_ctx_t *pctx) {
   if (pctx->of->w_kc && strcmp(pctx->tok->run, KEYWRD_SCENE)) {
      pctx->reason = "this keyword should be 'Scene'";
      synwarn(pctx);
   }

   pctx->scene = graft_tree_n(
      pctx->act, 0, NODEKIND_SCENE, pctx->tok
   );

   pctx->reason = msgs.err.syn.scene.incomp;
   gettok(pctx);

   if (!is_rnum(pctx->tok->run)) {
      graft_tree_s(pctx->scene, NODEKIND__ERROR, pctx->tok);
      pctx->reason = msgs.err.syn.scene.nornum;
      regerr(pctx);
      resync(pctx, ":");
   }
   else {
      graft_tree_s(pctx->scene, NODEKIND_ROMNUM, pctx->tok);
      gettok(pctx);
   }

   if (pctx->tok->run[0] != ':') {
      pctx->reason = msgs.err.syn.scene.nocolon;
      regerr(pctx);
   }

   pctx->reason = msgs.err.syn.scene.desc_incomp;
   skiptoks(pctx, ".!?");
}

static void seek_stmt(parse_ctx_t *pctx) {
   check_eoe(pctx);

   pctx->reason = msgs.err.syn.eot;
   gettok(pctx);

   seek_stmt_router(pctx);

   pctx->reason = msgs.err.syn.incomprehensible;
   synerr(pctx);
}

static void seek_stmt_router(parse_ctx_t *pctx) {
   typedef struct jumper {
      seeker_t *seek;
      int retval;
   } jumper_t;

   static const jumper_t jps[] = {
      { seek_line   , NODEKIND_LINE   },
      { seek_enter  , NODEKIND_ENTER  },
      { seek_exit   , NODEKIND_EXIT   },
      { seek_scene  , NODEKIND_SCENE  },
      { seek_act    , NODEKIND_ACT    },
      { seek_exeunt , NODEKIND_EXEUNT }
   };
   static const size_t jps_len = ARRLEN(jps);
   const jumper_t *jp;

   /* Since backtracking can happen, we need to save the
      current parsing state */
   const size_t orig_idx = pctx->idx;

   for (size_t i = 0; i < jps_len; i++) {
      jp = &jps[i];
      if ((*jp->seek)(pctx)) {
         longjmp(pctx->env, jp->retval);
      }
      rewind_tokstate(pctx, orig_idx);
   }
}

static int parse_stmt(parse_ctx_t *pctx) {
   int ret;

   /* seek_stmt proceeds to either
         - seek_stmt_router => longjmp
         - longjmp with NODEKIND__FINALE

      parse_line => seek_stmt_router => longjmp */

   ret = setjmp(pctx->env);
   switch (ret) {
      case NODEKIND_ENTER  : parse_enter (pctx); break;
      case NODEKIND_EXIT   : parse_exit  (pctx); break;
      case NODEKIND_EXEUNT : parse_exeunt(pctx); break;
      case NODEKIND_LINE   : parse_line  (pctx); break;
      case NODEKIND_SCENE   : /* fall-through */
      case NODEKIND_ACT     : /* fall-through */
      case NODEKIND__FINALE : return ret;
      case NODEKIND__SETJMP : break;  /* first setjmp call */
      default: ;  /* control never reaches default */
   }
   seek_stmt(pctx);

   return 0;  /* control never reaches here */
}

static int seek_enterlike(parse_ctx_t *pctx, const char *type) {
   if (pctx->tok->run[0] != '[') {
      return 0;
   }

   gettok(pctx);

   if (strcasecmp(pctx->tok->run, type)) {
      return 0;
   }

   return 1;
}

static int seek_enter(parse_ctx_t *pctx) {
   return seek_enterlike(pctx, KEYWRD_ENTER);
}

static void parse_enter(parse_ctx_t *pctx) {
   tree_t *enter;

   if (pctx->of->w_kc && strcmp(pctx->tok->run, KEYWRD_ENTER)) {
      pctx->reason = "this keyword should be 'Enter'";
      synwarn(pctx);
   }

   enter = graft_tree_n(pctx->scene, 0, NODEKIND_ENTER, pctx->tok);
   parse_namelist(pctx, enter);
   if (tree_clen(enter))
      return;
   pctx->reason = msgs.err.syn.enter.nochar;
   synerr(pctx);
}

static int seek_exit(parse_ctx_t *pctx) {
   return seek_enterlike(pctx, KEYWRD_EXIT);
}

static void parse_exit(parse_ctx_t *pctx) {
   tree_t *exit;

   if (pctx->of->w_kc && strcmp(pctx->tok->run, KEYWRD_EXIT)) {
      pctx->reason = "this keyword should be 'Exit'";
      synwarn(pctx);
   }

   exit = graft_tree_n(pctx->scene, 0, NODEKIND_EXIT, pctx->tok);
   parse_namelist(pctx, exit);
   switch (tree_clen(exit)) {
      case 0  : pctx->reason = msgs.err.syn.exit.nochar; break;
      case 1  : return;
      default : pctx->reason = msgs.err.syn.exit.exceed;
   }
   synerr(pctx);
}

static int seek_exeunt(parse_ctx_t *pctx) {
   return seek_enterlike(pctx, KEYWRD_EXEUNT);
}

static void parse_exeunt(parse_ctx_t *pctx) {
   tree_t *exeunt;

   if (pctx->of->w_kc && strcmp(pctx->tok->run, KEYWRD_EXEUNT)) {
      pctx->reason = "this keyword should be 'Exeunt'";
      synwarn(pctx);
   }

   exeunt = graft_tree_n(pctx->scene, 0, NODEKIND_EXEUNT, pctx->tok);
   parse_namelist(pctx, exeunt);
   switch (tree_clen(exeunt)) {
      case 0  : return;
      case 1  : pctx->reason = msgs.err.syn.exeunt.onechar; break;
      case 2  : return;
      default : pctx->reason = msgs.err.syn.exeunt.exceed;
   }
   synerr(pctx);
}

static int seek_line(parse_ctx_t *pctx) {
   return is_name(pctx);
}

static void parse_line(parse_ctx_t *pctx) {
   pctx->reason = msgs.err.syn.line.incomp;
   gettok(pctx);

   if (pctx->tok->run[0] != ':') {
      pctx->reason = "a colon expected after a character name to construct a line";
      synerr(pctx);
   }

   /* Note. charidx has been updated by is_name() in seek_line() */
   pctx->line = graft_tree_n(pctx->scene, 0, NODEKIND_LINE, pctx->tok);
   graft_tree_n(pctx->line, pctx->charidx, NODEKIND_CHAR, pctx->tok);

   /* Handles the first statement */
   pctx->reason = msgs.err.syn.line.incomp;
   gettok(pctx);

   if (parse_line_router(pctx, stmt_hdlrs, ARRLEN(stmt_hdlrs))) {
      pctx->reason = msgs.err.syn.line.nostmt;
      synerr(pctx);
   }

   /* Handles the rest */
   for (;;) {
      check_eoe(pctx);

      /* if this token is the beginning of another line,
         then longjmp happens inside `seek_stmt_router` */
      pctx->reason = msgs.err.syn.eot;
      gettok(pctx);

      seek_stmt_router(pctx);

      if (parse_line_router(pctx, stmt_hdlrs, ARRLEN(stmt_hdlrs))) {
         pctx->reason = msgs.err.syn.line.nostmt;
         synerr(pctx);
      }
   }
}

static bool parse_line_router(
   parse_ctx_t *pctx, const stmthandler_t *table, size_t tsiz
) {
   const stmthandler_t *handler;
   size_t i;

   /* Since backtracking can happen, we need to save the
      current parsing state */
   const size_t orig_idx = pctx->idx;

   for (i = 0; i < tsiz; i++) {
      handler = &table[i];
      if ((*handler->seek)(pctx)) {
         (*handler->parse)(pctx);
         break;
      }
      rewind_tokstate(pctx, orig_idx);
   }

   return (i == tsiz) ? 1 : 0;
}

static bool parse_line_as_conseq(parse_ctx_t *pctx) {
   const stmthandler_t *table = stmt_hdlrs;
   size_t tsiz = ARRLEN(stmt_hdlrs);

   pctx->reason = msgs.err.syn.ifstmt.conseq_incomp;
   gettok(pctx);

   if (isupper(pctx->tok->run[0])) {
      pctx->reason = msgs.err.syn.ifstmt.conseq_cap;
      synerr(pctx);
   }
   else
   if (islower(pctx->tok->run[0])) {
      pctx->tok->run[0] = toupper(pctx->tok->run[0]);
   }

   /* the standard says 'if' statement can't have an 'if' statement as a consequent */
   if (!strcmp(pctx->ov->std, stdopt_spl01)) {
      table = &stmt_hdlrs[1];
      tsiz--;
   }

   return parse_line_router(pctx, table, tsiz);
}

static int seek_if(parse_ctx_t *pctx) {
   return strcasecmp(pctx->tok->run, KEYWRD_IF) ? 0 : 1;
}

static void parse_if(parse_ctx_t *pctx) {
   tree_t *ifstmt, *consequent, *tline;

   ifstmt = graft_tree_n(pctx->line, 0, NODEKIND_IF, pctx->tok);

   pctx->reason = msgs.err.syn.ifstmt.incomp;
   gettok(pctx);

   if (!strcasecmp(pctx->tok->run, KEYWRD_SO)) {
      graft_tree_n(ifstmt, 0, NODEKIND_AFFIRM, pctx->tok);
   }
   else
   if (!strcasecmp(pctx->tok->run, KEYWRD_NOT)) {
      graft_tree_n(ifstmt, 0, NODEKIND_NEGATE, pctx->tok);
   }
   else {
      pctx->reason = msgs.err.syn.ifstmt.badsyn;
      synerr(pctx);
   }

   gettok(pctx);

   if (pctx->tok->run[0] != ',') {
      pctx->reason = msgs.err.syn.ifstmt.badsyn;
      synerr(pctx);
   }

   /* Parses the consequent */
   consequent = graft_tree_n(ifstmt, 0, NODEKIND_CONSEQ, pctx->tok);
   tline = pctx->line;
   pctx->line = consequent;

   if (parse_line_as_conseq(pctx)) {
      pctx->reason = msgs.err.syn.ifstmt.bad_conseq;
      synerr(pctx);
   }

   pctx->line = tline;
}

static int seek_asgn(parse_ctx_t *pctx) {
   if (!strcasecmp(pctx->tok->run, KEYWRD_YOU)) {
      return 1;
   }

   if (!strcasecmp(pctx->tok->run, KEYWRD_THOU)) {
      return 1;
   }

   return 0;
}

static void parse_asgn(parse_ctx_t *pctx) {
   /* TYPE 1: You A(.|!)
      TYPE 2: You be as adj as (B|C|D)(.|!)
      TYPE 3: You be (B|C|D)(.|!) */

   tree_t *asgn;
   token_t *you;

   /* It's obvious that the current tok->run is either
      "you" or "thou" because of `seek_asgn()` */
   you = pctx->tok;

   pctx->reason = msgs.err.syn.asgn.incomp;
   gettok(pctx);

   if (is_be_conjs(pctx)) {  /* type 2 or 3 */
      gettok(pctx);

      if (!strcasecmp(pctx->tok->run, KEYWRD_AS)) {
         asgn = parse_asgn_ii(pctx, you);  /* type 2 */
      }
      else {
         ungettok(pctx);
         asgn = parse_asgn_iii(pctx, you);  /* type 3 */
      }
   }
   else {  /* type 1 */
      ungettok(pctx);
      asgn = parse_asgn_i(pctx, you);
   }

   parse_const(pctx, asgn);

   if (strchr(".!", pctx->tok->run[0])) {
      return;
   }

   pctx->reason = msgs.err.syn.asgn.invalid_end_symbol;
   synerr(pctx);
}

static tree_t *parse_asgn_i(parse_ctx_t *pctx, token_t *you) {
   /* TYPE 1: You A(.|!) */
   return graft_tree_s(pctx->line, NODEKIND_ASGN1, you);
}

static tree_t *parse_asgn_ii(parse_ctx_t *pctx, token_t *you) {
   /* TYPE 2: You be as adj as (B|C|D)(.|!) */

   if (pctx->of->w_bp) {
      check_asgn_predicate(pctx);
   }

   pctx->reason = msgs.err.syn.asgn.incomp;
   gettok(pctx);

   if (!query_adj_lower(pctx->tok->run)) {
      pctx->reason = msgs.err.syn.asgn.not_adj;
      synerr(pctx);
   }

   pctx->reason = msgs.err.syn.asgn.incomp;
   gettok(pctx);

   if (strcasecmp(pctx->tok->run, KEYWRD_AS)) {
      pctx->reason = msgs.err.syn.asgn.not_as;
      synerr(pctx);
   }

   return graft_tree_s(pctx->line, NODEKIND_ASGN2, you);
}

static tree_t *parse_asgn_iii(parse_ctx_t *pctx, token_t *you) {
   /* TYPE 3: You be (B|C|D)(.|!) */

   if (pctx->of->w_bp) {
      check_asgn_predicate(pctx);
   }

   return graft_tree_s(pctx->line, NODEKIND_ASGN3, you);
}

static int seek_out(parse_ctx_t *pctx) {
   /* Speak your mind! */
   if (!strcasecmp(pctx->tok->run, KEYWRD_SPEAK)) {
      return 1;
   }

   /* Open your heart! */
   if (!strcasecmp(pctx->tok->run, KEYWRD_OPEN)) {
      gettokn(pctx, 2);  /* skips "your" */
      if (strcasecmp(pctx->tok->run, KEYWRD_HEART)) {
         return 0;
      }
      ungettokn(pctx, 2);
      return 1;
   }

   return 0;
}

static void parse_out(parse_ctx_t *pctx) {
   nodekind_t kind;
   bool test1, test2, test3;

   if (!strcasecmp(pctx->tok->run, KEYWRD_OPEN)) {
      kind = NODEKIND_OUT_N;
   }
   else {  /* Speak */
      kind = NODEKIND_OUT_C;
   }

   graft_tree_n(pctx->line, 0, kind, pctx->tok);

   pctx->reason = msgs.err.syn.out.incomp;
   gettok(pctx);

   test1 = !strcasecmp(pctx->tok->run, KEYWRD_YOUR)
      || !strcasecmp(pctx->tok->run, KEYWRD_THINE)
      || !strcasecmp(pctx->tok->run, KEYWRD_THY);

   if (!test1) {
      pctx->reason = msgs.err.syn.out.badsyn;
      synerr(pctx);
   }

   gettok(pctx);

   test2 = (kind == NODEKIND_OUT_N) && (!strcasecmp(pctx->tok->run, KEYWRD_MIND));
   test3 = (kind == NODEKIND_OUT_C) && (!strcasecmp(pctx->tok->run, KEYWRD_HEART));

   if (test2 || test3) {
      pctx->reason = msgs.err.syn.out.unmatched;
      synerr(pctx);
   }

   gettok(pctx);

   if (!match(pctx->tok->run[0], ".!")) {
      pctx->reason = msgs.err.syn.out.badsyn;
      synerr(pctx);
   }
}

static int seek_in(parse_ctx_t *pctx) {
   /* Listen to your heart! */
   if (!strcasecmp(pctx->tok->run, KEYWRD_LISTEN)) {
      return 1;
   }

   /* Open your mind! */
   if (!strcasecmp(pctx->tok->run, KEYWRD_OPEN)) {
      gettokn(pctx, 2);  /* skips "your" */
      if (strcasecmp(pctx->tok->run, KEYWRD_MIND)) {
         return 0;
      }
      ungettokn(pctx, 2);
      return 1;
   }

   return 0;
}

static void parse_in(parse_ctx_t *pctx) {
   nodekind_t kind;
   bool test1, test2, test3;

   if (!strcasecmp(pctx->tok->run, KEYWRD_LISTEN)) {
      kind = NODEKIND_IN_N;
   }
   else  { /* Open */
      kind = NODEKIND_IN_C;
   }

   graft_tree_n(pctx->line, 0, kind, pctx->tok);
   pctx->reason = msgs.err.syn.in.incomp;

   if (kind == NODEKIND_IN_N) {
      gettok(pctx);
      if (strcasecmp(pctx->tok->run, KEYWRD_TO)) {
         pctx->reason = msgs.err.syn.in.badsyn;
         synerr(pctx);
      }
   }

   gettok(pctx);

   test1 = !strcasecmp(pctx->tok->run, KEYWRD_YOUR)
      || !strcasecmp(pctx->tok->run, KEYWRD_THINE)
      || !strcasecmp(pctx->tok->run, KEYWRD_THY);

   if (!test1) {
      pctx->reason = msgs.err.syn.in.badsyn;
      synerr(pctx);
   }

   gettok(pctx);

   test2 = (kind == NODEKIND_OUT_N) && (!strcasecmp(pctx->tok->run, KEYWRD_MIND));
   test3 = (kind == NODEKIND_OUT_C) && (!strcasecmp(pctx->tok->run, KEYWRD_HEART));

   if (test2 || test3) {
      pctx->reason = msgs.err.syn.in.unmatched;
      synerr(pctx);
   }

   gettok(pctx);

   if (!match(pctx->tok->run[0], ".!")) {
      pctx->reason = msgs.err.syn.in.badsyn;
      synerr(pctx);
   }
}

static int seek_goto(parse_ctx_t *pctx) {
   if (strcasecmp(pctx->tok->run, KEYWRD_LET) && strcasecmp(pctx->tok->run, KEYWRD_WE)) {
      return 0;
   }
   return 1;
}

static void parse_goto(parse_ctx_t *pctx) {
   enum { GototypeLet, GototypeWe } type;
   nodekind_t dest;
   bool test1, test2, test3, test4;
   tree_t *gt;

   if (!strcasecmp(pctx->tok->run, KEYWRD_LET)) {
      type = GototypeLet;
   }
   else {  /* we */
      type = GototypeWe;
   }

   pctx->reason = msgs.err.syn.gt.incomp;
   gettok(pctx);

   test1 = !strcasecmp(pctx->tok->run, KEYWRD_US);
   test2 = !strcasecmp(pctx->tok->run, KEYWRD_SHALL)
           || !strcasecmp(pctx->tok->run, KEYWRD_MUST);

   if (!test1 && !test2) {
      pctx->reason = msgs.err.syn.gt.badsyn;
      synerr(pctx);
   }

   test1 = (type == GototypeWe) && test1;
   test2 = (type == GototypeLet) && test2;

   if (test1 || test2) {
      pctx->reason = msgs.err.syn.gt.unmatched;
      synerr(pctx);
   }

   gettok(pctx);

   test3 = strcasecmp(pctx->tok->run, KEYWRD_RETURN);
   test4 = strcasecmp(pctx->tok->run, KEYWRD_PROCED);

   if (test3 && test4) {
      pctx->reason = msgs.err.syn.gt.badsyn;
      synerr(pctx);
   }

   gettok(pctx);

   if (strcasecmp(pctx->tok->run, KEYWRD_TO)) {
      pctx->reason = msgs.err.syn.gt.badsyn;
      synerr(pctx);
   }

   gettok(pctx);

   if (!strcasecmp(pctx->tok->run, KEYWRD_ACT)) {
      dest = NODEKIND_ACT;
   }
   else
   if (!strcasecmp(pctx->tok->run, KEYWRD_SCENE)) {
      dest = NODEKIND_SCENE;
   }
   else {
      pctx->reason = msgs.err.syn.gt.badsyn;
      synerr(pctx);
   }

   gt = graft_tree_n(pctx->line, dest, NODEKIND_GOTO, pctx->tok);

   gettok(pctx);

   if (pctx->tok->kind == TOKKIND_PNT) {
      pctx->reason = msgs.err.syn.gt.badsyn;
      synerr(pctx);
   }

   // fixme: 로마숫자인지 검사
   graft_tree_s(gt, NODEKIND_ROMNUM, pctx->tok);

   gettok(pctx);

   if (!match(pctx->tok->run[0], ".!")) {
      pctx->reason = msgs.err.syn.gt.badsyn;
      synerr(pctx);
   }
}

static int seek_cond(parse_ctx_t *pctx) {
   return is_be_conjs(pctx);
}

static void parse_cond(parse_ctx_t *pctx) {
   tree_t *condition,
          *lefthand,
          *righthand,
          *p;
   nodekind_t kind;

   /* Note: [x] = current token
      type 0: [Am] I not better than yourself?
      type 2: [Art] thou not more cunning than the Ghost?
      type 3: [Is] a tree not as good as a shiny tree? */

   condition = graft_tree_n(pctx->line, 0, NODEKIND_COND, pctx->tok);
   lefthand  = graft_tree_n(condition, 0, NODEKIND_LHS, pctx->tok);

   pctx->reason = msgs.err.syn.cond.incomp;
   gettok(pctx);

   /* Am [I] not better than yourself?
      Art [thou] not more cunning than the Ghost?
      Is [a] tree not as good as a shiny tree? */

   /* checks the relation between the subject and the be verb conjugation
      only if --Wbe-predicate is activated */
   if (pctx->of->w_bp) {
      check_cond_predicate(pctx);
   }

   if (pctx->be_kind == 3) {
      /* need to ungettok before parse_const() */
      ungettok(pctx);
      /* [Is] a tree not as good as a shiny tree? */
   }

   switch (pctx->be_kind) {
      case 0  : kind = NODEKIND_P1; break;
      case 1  : /* fall-through */
      case 2  : kind = NODEKIND_P2; break;
      case 3  : kind = NODEKIND_P3; break;
      /* control never reaches default */
      default : kind = NODEKIND__UNKNOWN;
   }

   p = graft_tree_n(lefthand, 0, kind, pctx->tok);

   if (pctx->be_kind == 3) {
      /* parse_const() has consumed "a tree" */
      parse_const(pctx, p);
      /* Is a tree [not] as good as a shiny tree? */
   } else {
      pctx->reason = msgs.err.syn.cond.incomp;
      gettok(pctx);
      /* Am I [not] better than yourself?
         Art thou [not] more cunning than the Ghost? */
   }

   if (!strcasecmp(pctx->tok->run, KEYWRD_NOT)) {
      graft_tree_n(condition, 0, NODEKIND_NEGATE, pctx->tok);
      pctx->reason = msgs.err.syn.cond.incomp;
      gettok(pctx);
      /* Am I not [better] than yourself?
         Art thou not [more] cunning than the Ghost?
         Is a tree not [as] good as a shiny tree? */
   }
   else
      graft_tree_n(condition, 0, NODEKIND_AFFIRM, pctx->tok);

   if (!strcasecmp(pctx->tok->run, KEYWRD_AS)) {
      parse_cond_eq(pctx, condition);
      /* Is a tree not as good [as] a shiny tree? */
   }
   else {
      parse_cond_ineq(pctx, condition);
      /* Am I not better [than] yourself?
         Art thou not more cunning [than] the Ghost? */
   }

   righthand = graft_tree_n(condition, 0, NODEKIND_RHS, pctx->tok);
   /* The last noun is to be comsumed */
   parse_const(pctx, righthand);
   /* Am I not better than yourself[?]
      Art thou not more cunning than the Ghost[?]
      Is a tree not as good as a shiny tree[?] */
}

static void parse_cond_eq(parse_ctx_t *pctx, tree_t *cond) {
   /* Is a tree not [as] good as a shiny tree? */
   gettok(pctx);
   /* Is a tree not as [good] as a shiny tree? */

   if (pctx->tok->kind == TOKKIND_PNT) {
      pctx->reason = msgs.err.syn.cond.badsyn;
      synerr(pctx);
   }

   graft_tree_s(cond, NODEKIND_EQ, pctx->tok);

   gettok(pctx);
   /* Is a tree not as good [as] a shiny tree? */

   if (strcasecmp(pctx->tok->run, KEYWRD_AS)) {
      pctx->reason = msgs.err.syn.cond.badsyn;
      synerr(pctx);
   }
}

static void parse_cond_ineq(parse_ctx_t *pctx, tree_t *cond) {
   static const char *comps[2] = { KEYWRD_MORE, KEYWRD_LESS };
   static const int comps_len = ARRLEN(comps);

   int ret;
   nodekind_t kind;

   /* Art thou not [more] cunning than the Ghost?
      Am I not [better] than yourself? */

   ret = match_str_case(pctx->tok->run, comps, comps_len);

   /* like "more beautiful" or "less interesting" */
   if (ret < comps_len) {
      pctx->reason = msgs.err.syn.cond.incomp;
      gettok(pctx);

      /* Art thou not more [cunning] than the Ghost? */

      if (pctx->tok->kind == TOKKIND_PNT) {
         pctx->reason = msgs.err.syn.cond.badsyn;
         synerr(pctx);
      }

      switch (ret) {
         case 0 : kind = NODEKIND_GT; break;
         case 1 : kind = NODEKIND_LT; break;
         default: ;  /* control never reaches default */
      }

      graft_tree_s(cond, kind, pctx->tok);
   }
   /* one word comparative like "better" or "worse" */
   else {
      graft_tree_s(cond, NODEKIND_INEQ, pctx->tok);
   }

   pctx->reason = msgs.err.syn.cond.incomp;
   gettok(pctx);
   /* Am I not better [than] yourself?
      Art thou not more cunning [than] the Ghost? */

   if (strcasecmp(pctx->tok->run, KEYWRD_THAN)) {
      pctx->reason = msgs.err.syn.cond.badsyn;
      synerr(pctx);
   }
}

static int seek_push(parse_ctx_t *pctx) {
   return strcasecmp(pctx->tok->run, KEYWRD_REMEMB) ? 0 : 1;
}

static void parse_push(parse_ctx_t *pctx) {
   tree_t *push;

   push = graft_tree_n(pctx->line, 0, NODEKIND_PUSH, pctx->tok);

   pctx->reason = msgs.err.syn.push.incomp;
   gettok(pctx);

   if (pctx->tok->kind == TOKKIND_PNT) {
      pctx->reason = msgs.err.syn.push.badsyn;
      synerr(pctx);
   }

   ungettok(pctx);

   parse_const(pctx, push);

   if (!match(pctx->tok->run[0], ".!")) {
      pctx->reason = msgs.err.syn.push.badsyn;
      synerr(pctx);
   }
}

static int seek_pop(parse_ctx_t *pctx) {
   return strcasecmp(pctx->tok->run, KEYWRD_RECALL) ? 0 : 1;
}

static void parse_pop(parse_ctx_t *pctx) {
   graft_tree_n(pctx->line, 0, NODEKIND_POP, pctx->tok);
   pctx->reason = msgs.err.syn.pop.incomp;
   skiptoks(pctx, ".!?");
}

static void parse_namelist(parse_ctx_t *pctx, tree_t *t) {
   /* Enter, Exit, and Exeunt takes a namelist:
         (1) [<enterlike> A]
         (2) [<enterlike> A and B]
         (3) [<enterlike> A, B, and C] */

   pctx->reason = "incomplete namelist";
   gettok(pctx);
   if (pctx->tok->run[0] == ']')
         return;
   ungettok(pctx);

   /* Currently, tok->run points to <enterlike> */

   /* Consumes tokens until "and" */
   do {
      pctx->reason = "incomplete namelist"; // fixme: remove this
      gettok(pctx);
      if (!is_name_lower(pctx)) {
         pctx->reason = "dp expected here";
         synerr(pctx);
      }
      graft_tree_n(t, pctx->charidx, NODEKIND_CHAR, pctx->tok);
      gettok(pctx);
      if (pctx->tok->run[0] == ']')
         return;
      if (!strcasecmp(pctx->tok->run, KEYWRD_AND))
         break;
      if (pctx->tok->run[0] != ',') {
         pctx->reason = ", expected here";
         synerr(pctx);
      }
      gettok(pctx);
      if (!strcasecmp(pctx->tok->run, KEYWRD_AND))
         break;
      ungettok(pctx);
   } while (true);

   /* Skips "and" and consumes the last name */
   gettok(pctx);
   if (!is_name_lower(pctx)) {
      pctx->reason = "dp expected here";
      synerr(pctx);
   }
   graft_tree_n(t, pctx->charidx, NODEKIND_CHAR, pctx->tok);
   gettok(pctx);
   if (pctx->tok->run[0] != ']') {
      pctx->reason = "] expected here";
      synerr(pctx);
   }
}

static void parse_const(parse_ctx_t *pctx, tree_t *stmt) {
   /* Before going further, let's recall where constants are used.

      Note:
         - <x> means x is optional.
         - (x|y) means x or y must be present.
         - "ap" means adjective phrase: "happy delightful nice".
         - "np" means noun phrase: "summer's day", "stone wall".
            - incidentally, a noun phrase is treated as a single noun!
         - "art" means articles: "a", "an", or "the".
         - "pos" means possessives: "my", "your", etc.
         - "pronoun": "me", "you", etc.
         - "ref" means reflexives: "myself", "yourself", etc.
         - "comp" means comparatives: "better", "worse", etc

      Constants can have one of the following forms:
         TYPE A: <adj|ap> (noun|np)
         TYPE B: (art|pos) <adj|ap> (noun|np)  // i.e. (art|pos) A
         TYPE C: (pronoun|ref|name)
         TYPE D: (nothing|zero)

      1. Assignment Statements
         You A(.|!)
         You be as adj as (B|C|D)(.|!)
         You be (B|C|D)(.|!)

      2. Questions
         Be (B|C|D) <not> as adj as (B|C|D)?
         Be (B|C|D) <not> (comp|(<more|less> adj)) than (B|C|D)?

      3. As Operands Of Operators
         the factorial of (B|C|D)
         the sum of (B|C|D) and (B|C|D)

      4. Remember Statements
         Remember (B|C|D)(.|!)

      This remind shows us that TYPE A is only used in the
      "YOU A" assignment statement. Also, the combination of
      B, C, and D is widely used.

      With that in mind, now let's begin parsing. */

   tree_t *cnst;
   nodekind_t kind;
   int query_result;

   /* Makes a tree that represents a constant node */
   cnst = graft_tree_n(stmt, 0, NODEKIND_CONST, pctx->tok);

   pctx->reason = msgs.err.syn.cnst.incomp;
   gettok(pctx);

   /* First of all, we check whether this is an operator */
   if ((kind = seek_op(pctx)) != NODEKIND__NAO) {
      parse_op(pctx, cnst, kind);
      return;
   }

   /* Checks whether this token is
      a pronoun, a reflexive, a name, or a nil */
   if (is_pronoun(pctx->tok->run)) {
      graft_tree_s(cnst, what_pronoun(pctx->tok->run), pctx->tok);
      check_const_end(pctx);
      return;
   }
   if (is_reflexive(pctx->tok->run)) {
      graft_tree_s(cnst, what_reflexive(pctx->tok->run), pctx->tok);
      check_const_end(pctx);
      return;
   }
   if (is_name_lower(pctx)) {
      graft_tree_n(cnst, pctx->charidx, NODEKIND_CHAR, pctx->tok);
      check_const_end(pctx);
      return;
   }
   if (is_nil(pctx->tok->run)) {
      graft_tree_s(cnst, NODEKIND_ZERO, pctx->tok);
      check_const_end(pctx);
      return;
   }

   /* If not, this token is either TYPE A or TYPE B. Meanwhile,
      TYPE B = (art|pos) TYPE A. Let's exploit this structure */
   if (is_article(pctx->tok->run) || is_possessive(pctx->tok->run)) {
      pctx->reason = msgs.err.syn.cnst.incomp;
      gettok(pctx);  /* skips the current token */
      /* if the token had been of TYPE B, now it has become of TYPE A */
   }

   /* Now we need to process <adj|ap> (noun|np) */

   /* Consumes adjectives first */
   for (;;) {
      if (!query_adj_lower(pctx->tok->run))
         break;
      graft_tree_s(cnst, NODEKIND_ADJ, pctx->tok);
      gettok(pctx);
   }

   /* noun */
   if (query_noun(pctx->tok->run, &query_result)) {
      kind = query_result ? NODEKIND_PNOUN : NODEKIND_NNOUN;
      graft_tree_s(cnst, kind, pctx->tok);
   }
   /* noun phrase */
   else {
      parse_noun_phrase(pctx, cnst);
   }

   /* end of const */
   check_const_end(pctx);
}

static void parse_noun_phrase(parse_ctx_t *pctx, tree_t *cnst) {
   token_t *prev_tok;
   char *buf;
   size_t bufsiz;
   nodekind_t kind;
   int query_result;
   token_t tok;

   prev_tok = pctx->tok;

   pctx->reason = msgs.err.syn.cnst.incomp;
   gettok(pctx);

   /* EXAMPLE
      prev = "stone\0"  (len = 5)
      tok  = "wall\0"   (len = 4)
      buf  = "stone wall\0"  (len = 10) */

   bufsiz = prev_tok->len + 1 + pctx->tok->len;
   buf = safe_malloc(bufsiz + 1);  /* +1 for \0 */
   memcpy(
      buf,
      prev_tok->run,
      prev_tok->len
   );
   buf[prev_tok->len] = ' ';
   memcpy(
      buf + prev_tok->len + 1,
      pctx->tok->run,
      pctx->tok->len + 1  /* \0 copied */
   );

   if(!query_noun(buf, &query_result)) {
      ungettok(pctx);
      pctx->reason = msgs.err.syn.cnst.no_noun;
      synerr(pctx);
   }

   kind = query_result ? NODEKIND_PNOUN : NODEKIND_NNOUN;

   tok.run = buf;
   tok.len = bufsiz;
   tok.lnum = pctx->tok->lnum;
   tok.lpos = pctx->tok->lpos;

   graft_tree_s(cnst, kind, &tok);

   free(buf);
}

static nodekind_t seek_op(parse_ctx_t *pctx) {
   typedef struct ophandler {
      const char *name;
      nodekind_t kind;
   } ophandler_t;

   static ophandler_t ops[] = {
      { KEYWRD_SUM  , NODEKIND_SUM  },
      { KEYWRD_DIFF , NODEKIND_DIFF },
      { KEYWRD_PROD , NODEKIND_PROD },
      { KEYWRD_QUOT , NODEKIND_QUOT },
      { KEYWRD_REM  , NODEKIND_REM  },
      { KEYWRD_SQUR , NODEKIND_SQUR },
      { KEYWRD_CUBE , NODEKIND_CUBE },
      { KEYWRD_FACT , NODEKIND_FACT }
   };
   static const size_t ops_len = ARRLEN(ops);

   /* Note:
      - except the twice operator,
         every operator begins with "the"
      - except the square root operator,
         every operator is one word long */

   nodekind_t k;

   /* twice operator? */
   if (!strcasecmp(pctx->tok->run, KEYWRD_2X))
      return NODEKIND_2X;

   /* not begins with "the"? then it's not an operator */
   if (strcasecmp(pctx->tok->run, KEYWRD_THE))
      return NODEKIND__NAO;

   pctx->reason = msgs.err.syn.cnst.incomp;
   gettok(pctx);

   k = NODEKIND__NAO;
   for (size_t i = 0; i < ops_len; i++)
      if (!strcasecmp(pctx->tok->run, ops[i].name)) {
         k = ops[i].kind;
         break;
      }

   /* no match! turns out it isn't an operator,
      although it began with "the" */
   if (k == NODEKIND__NAO) {
      ungettok(pctx);
      return k;
   }

   /* not a square or a square root operator */
   if (k != NODEKIND_SQUR)
      return k;

   /* to be a square or to be a square root? that's the question */
   gettok(pctx);
   if (!strcasecmp(pctx->tok->run, KEYWRD_ROOT))
      k = NODEKIND_SQRT;
   else ungettok(pctx);

   return k;
}

static void parse_op(parse_ctx_t *pctx, tree_t *stmt, nodekind_t kind) {
   tree_t *op;

   op = graft_tree_n(stmt, 0, kind, pctx->tok);
   pctx->reason = msgs.err.syn.op.incomp;
   switch (kind) {
      case NODEKIND_SUM  : parse_op_sum (pctx, op); return;
      case NODEKIND_DIFF : parse_op_diff(pctx, op); return;
      case NODEKIND_PROD : parse_op_prod(pctx, op); return;
      case NODEKIND_QUOT : parse_op_quot(pctx, op); return;
      case NODEKIND_REM  : parse_op_rem (pctx, op); return;
      case NODEKIND_SQRT : parse_op_sqrt(pctx, op); return;
      case NODEKIND_SQUR : parse_op_squr(pctx, op); return;
      case NODEKIND_CUBE : parse_op_cube(pctx, op); return;
      case NODEKIND_2X   : parse_op_2x  (pctx, op); return;
      case NODEKIND_FACT : parse_op_fact(pctx, op); return;
      default: ;  /* control never reaches default */
   }
}

static void parse_op_unary(
   parse_ctx_t *pctx,
   tree_t * restrict op,
   const char * restrict err
) {
   /* Note. this function is a wrapper for the
      square, square root, cube, and factorial.

      Since the twice operator does not have the
      same structure with other unary operators,
      it doesn't use this wrapper. */

   pctx->reason = msgs.err.syn.op.incomp;
   gettok(pctx);

   if (strcasecmp(pctx->tok->run, KEYWRD_OF)) {
      pctx->reason = err;
      synerr(pctx);
   }

   parse_const(pctx, op);
}

static void parse_op_binary(
   parse_ctx_t *pctx,
   tree_t * restrict op,
   const char * restrict type,
   const char * restrict err
) {
   tree_t *lefthand, *righthand;

   pctx->reason = msgs.err.syn.op.incomp;
   gettok(pctx);

   /* the sum OF L and R
      the difference BETWEEN L and R
      the product OF L and R
      the quotient BETWEEN L and R
      the remainder OF the quotient BETWEEN L and R */

   if (strcasecmp(pctx->tok->run, type)) {
      pctx->reason = err;
      synerr(pctx);
   }

   lefthand = graft_tree_n(op, 0, NODEKIND_LHS, pctx->tok);
   parse_const(pctx, lefthand);

   if (strcasecmp(pctx->tok->run, KEYWRD_AND)) {
      pctx->reason = msgs.err.syn.op.no_and;
      synerr(pctx);
   }

   righthand = graft_tree_n(op, 0, NODEKIND_RHS, pctx->tok);
   parse_const(pctx, righthand);
}

static inline void parse_op_sum(parse_ctx_t *pctx, tree_t *op) {
   parse_op_binary(pctx, op, KEYWRD_OF, msgs.err.syn.op.sum);
}

static inline void parse_op_diff(parse_ctx_t *pctx, tree_t *op) {
   parse_op_binary(pctx, op, KEYWRD_BTW, msgs.err.syn.op.diff);
}

static inline void parse_op_prod(parse_ctx_t *pctx, tree_t *op) {
   parse_op_binary(pctx, op, KEYWRD_OF, msgs.err.syn.op.prod);
}

static inline void parse_op_quot(parse_ctx_t *pctx, tree_t *op) {
   parse_op_binary(pctx, op, KEYWRD_BTW, msgs.err.syn.op.quot);
}

static inline void parse_op_rem(parse_ctx_t *pctx, tree_t *op) {
   /* the remainder of
         the quotient between <const> and <const> */

   pctx->reason = msgs.err.syn.op.incomp;
   gettok(pctx);

   if (strcasecmp(pctx->tok->run, KEYWRD_OF)) {
      pctx->reason = msgs.err.syn.op.rem;
      synerr(pctx);
   }

   gettok(pctx);

   if (strcasecmp(pctx->tok->run, KEYWRD_THE)) {
      pctx->reason = msgs.err.syn.op.rem_quot_1;
      synerr(pctx);
   }

   gettok(pctx);

   if (strcasecmp(pctx->tok->run, KEYWRD_QUOT)) {
      pctx->reason = msgs.err.syn.op.rem_quot_2;
      synerr(pctx);
   }

   parse_op_quot(pctx, op);
}

static inline void parse_op_sqrt(parse_ctx_t *pctx, tree_t *op) {
   parse_op_unary(pctx, op, msgs.err.syn.op.sqrt);
}

static inline void parse_op_squr(parse_ctx_t *pctx, tree_t *op) {
   parse_op_unary(pctx, op, msgs.err.syn.op.squr);
}

static inline void parse_op_cube(parse_ctx_t *pctx, tree_t *op) {
   parse_op_unary(pctx, op, msgs.err.syn.op.cube);
}

static inline void parse_op_2x(parse_ctx_t *pctx, tree_t *op) {
   parse_const(pctx, op);
}

static inline void parse_op_fact(parse_ctx_t *pctx, tree_t *op) {
   parse_op_unary(pctx, op, msgs.err.syn.op.fact);
}

static bool is_pronoun(const char *str) {
   static const char *pronouns[] = {
      KEYWRD_I, KEYWRD_ME, KEYWRD_THEE, KEYWRD_THOU, KEYWRD_YOU, NULL
   };

   for (size_t i = 0; pronouns[i]; i++)
      if (!strcasecmp(str, pronouns[i]))
         return true;
   return false;
}

static bool is_reflexive(const char *str) {
   static const char *reflexives[] = {
      KEYWRD_MYSELF, KEYWRD_TYSELF, KEYWRD_YRSELF, NULL
   };

   for (size_t i = 0; reflexives[i]; i++)
      if (!strcasecmp(str, reflexives[i]))
         return true;
   return false;
}

static bool is_nil(const char *str) {
   static const char *nils[] = {
      KEYWRD_NOTHING, KEYWRD_ZERO, NULL
   };

   for (size_t i = 0; nils[i]; i++)
      if (!strcasecmp(str, nils[i]))
         return true;
   return false;
}

static bool is_article(const char *str) {
   static const char *articles[] = {
      KEYWRD_A, KEYWRD_AN, KEYWRD_THE, NULL
   };

   for (size_t i = 0; articles[i]; i++)
      if (!strcasecmp(str, articles[i]))
         return true;
   return false;
}

static bool is_possessive(const char *str) {
   static const char *possessives[] = {
      KEYWRD_MINE, KEYWRD_MY,
      KEYWRD_THINE, KEYWRD_THY, KEYWRD_YOUR,
      KEYWRD_HIS, KEYWRD_HER, KEYWRD_ITS, KEYWRD_THEIR, NULL
   };

   for (size_t i = 0; possessives[i]; i++)
      if (!strcasecmp(str, possessives[i]))
         return true;
   return false;
}

static nodekind_t what_pronoun(const char *str) {
   if (!strcasecmp(str, KEYWRD_I) || !strcasecmp(str, KEYWRD_ME))
      return NODEKIND_P1;

   if (!strcasecmp(str, KEYWRD_THEE)
      || !strcasecmp(str, KEYWRD_THOU)
      || !strcasecmp(str, KEYWRD_YOU))
      return NODEKIND_P2;

   /* control never reaches here */
   return NODEKIND__UNKNOWN;
}

static nodekind_t what_reflexive(const char *str) {
   if (!strcasecmp(str, KEYWRD_MYSELF))
      return NODEKIND_P1;

   if (!strcasecmp(str, KEYWRD_TYSELF) || !strcasecmp(str, KEYWRD_YRSELF))
      return NODEKIND_P2;

   /* control never reaches here */
   return NODEKIND__UNKNOWN;
}

static void check_const_end(parse_ctx_t *pctx) {
   pctx->reason = msgs.err.syn.cnst.incomp;
   gettok(pctx);

   if (strchr(".!?", pctx->tok->run[0])
      || !strcasecmp(pctx->tok->run, KEYWRD_NOT)
      || !strcasecmp(pctx->tok->run, KEYWRD_AS)
      || !strcasecmp(pctx->tok->run, KEYWRD_MORE)
      || !strcasecmp(pctx->tok->run, KEYWRD_LESS)
      || !strcasecmp(pctx->tok->run, KEYWRD_AND)
      || query_comp_lower(pctx->tok->run, NULL))
   {
      return;
   }

   pctx->reason = msgs.err.syn.cnst.no_end_symbol;
   synerr(pctx);
}

static bool is_name(parse_ctx_t *pctx) {
   /* DP  (siz = 2)
         => CHAR  (siz = 1)
            => Romeo
         => CHAR  (siz = 2)
            => The
            => Ghost   */

   tree_t *dp,  /* character list */
          *ch;  /* character */
   size_t dpsiz,  /* number of children of dp */
          chsiz;  /* number of children of char */
   node_t *ch_subnode;
   size_t i, k;

   /* Since backtracking can happen, we need to save the
      current parsing state */
   const size_t orig_idx = pctx->idx;

   /* We assume that all names are unique, i.e. there is
      no overlap like "the Romeo" and "the Romeo Rome" */

   dp = tree_child(pctx->pt, 1);
   dpsiz = tree_clen(dp);
   pctx->reason = "incomplete name";

   for (i = 0; i < dpsiz; i++) {
      ch = tree_child(dp, i);
      chsiz = tree_clen(ch);  /* can't be zero. see parse_dp() */

      for (k = 0; k < chsiz; k++) {
         ch_subnode = tree_chdat(ch, k);
         if (strcmp(ch_subnode->dat.s.run, pctx->tok->run)) {
            rewind_tokstate(pctx, orig_idx);
            break;
         }
         gettok(pctx);
      }

      if (k == chsiz) {
         ungettok(pctx);
         pctx->charidx = i;
         return true;
      }
   }

   return false;
}

static bool is_name_lower(parse_ctx_t *pctx) {
   if (!strcmp(pctx->tok->run, "A")
      || !strcmp(pctx->tok->run, "An")
      || !strcmp(pctx->tok->run, "The"))
   {
      pctx->reason = msgs.err.syn.name_not_lowcase;
      synerr(pctx);
   }

   if (!strcmp(pctx->tok->run, KEYWRD_A)
      || !strcmp(pctx->tok->run, KEYWRD_AN)
      || !strcmp(pctx->tok->run, KEYWRD_THE))
   {
      pctx->tok->run[0] = toupper(pctx->tok->run[0]);
   }

   if(is_name(pctx)) {
      return 1;
   }

   pctx->tok->run[0] = tolower(pctx->tok->run[0]);
   return 0;
}

static bool is_be_conjs(parse_ctx_t *pctx) {  /* conjs = conjugations */
   static const char *conjs[] = {
      "am", "are", "art", "is", "be"
   };
   static const size_t conjs_len = ARRLEN(conjs);

   pctx->be_kind = match_str_case(pctx->tok->run, conjs, conjs_len);

   return (pctx->be_kind < conjs_len) ? true : false;
}

static void check_cond_predicate(parse_ctx_t *pctx) {
   static const char *cond_subjs[] = {
      KEYWRD_I, KEYWRD_YOU, KEYWRD_THOU
   };
   static const size_t cond_subjs_len = ARRLEN(cond_subjs);

   switch (pctx->be_kind) {
      case 0 : if (strcasecmp(pctx->tok->run, KEYWRD_I))
                  goto hell; else break;
      case 1 : if (strcasecmp(pctx->tok->run, KEYWRD_YOU))
                  goto hell; else break;
      case 2 : if (strcasecmp(pctx->tok->run, KEYWRD_THOU))
                  goto hell; else break;
      /* i.e. check if "Is (I, you, thou)" */
      case 3 : if (match_str_case(pctx->tok->run, cond_subjs, cond_subjs_len) < cond_subjs_len)
                  goto hell; else break;
      hell: /* FLAMING HOT */
         ungettok(pctx);
         pctx->reason = msgs.err.syn.cond.not_conj;
         synwarn(pctx);
         gettok(pctx);
   }
}

static void check_asgn_predicate(parse_ctx_t *pctx) {
   /* are or art */
   if (pctx->be_kind == 1 || pctx->be_kind == 2) {
      return;
   }

   ungettok(pctx);
   pctx->reason = "be verb must be the second person in the assignment statement";
   synwarn(pctx);
   gettok(pctx);
}

static void nexttok(parse_ctx_t *pctx) {
   if (pctx->idx == pctx->len - 1) {
      regerr(pctx);
      longjmp(pctx->env, NODEKIND__FINALE);
   }

   pctx->idx++;
   pctx->tok = array_peek(pctx->toks, pctx->idx);
}

static void gettok(parse_ctx_t *pctx) {
   nexttok(pctx);
   pctx->etok = pctx->tok;
}

static void gettokn(parse_ctx_t *pctx, size_t n) {
   if (pctx->idx + n >= pctx->len) {
      regerr(pctx);
      longjmp(pctx->env, NODEKIND__FINALE);
   }

   pctx->idx += n;
   pctx->tok = array_peek(pctx->toks, pctx->idx);
   pctx->etok = pctx->tok;
}

static void ungettok(parse_ctx_t *pctx) {
   pctx->idx--;
   pctx->tok = array_peek(pctx->toks, pctx->idx);
   pctx->etok = pctx->tok;
}

static void ungettokn(parse_ctx_t *pctx, size_t n) {
   pctx->idx -= n;
   pctx->tok = array_peek(pctx->toks, pctx->idx);
   pctx->etok = pctx->tok;
}

static void skiptoks(parse_ctx_t *pctx, const char *sentinels) {
   for (;;) {
      nexttok(pctx);
      if (match(pctx->tok->run[0], sentinels)) {
         break;
      }
   }
   pctx->etok = pctx->tok;
}

static void eqtok(parse_ctx_t *pctx, char ch) {
   if (pctx->tok->run[0] != ch) {
      return;
   }
   pctx->etok = pctx->tok;
   synerr(pctx);
}

static void neqtok(parse_ctx_t *pctx, char ch) {
   if (pctx->tok->run[0] == ch) {
      return;
   }
   pctx->etok = pctx->tok;
   synerr(pctx);
}

static void readtoks(parse_ctx_t *pctx, char sentinel, tree_t *base) {
   for (;;) {
      nexttok(pctx);
      if (pctx->tok->run[0] == sentinel) {
         break;
      }
      graft_tree_s(base, NODEKIND_DATA, pctx->tok);
   }

   pctx->etok = pctx->tok;
}

static void readtoks_until(parse_ctx_t *pctx, char *scanset, tree_t *t) {
   for (;;) {
      nexttok(pctx);

      for (size_t i = 0; (scanset[i] != '\0'); i++) {
         if (pctx->tok->run[0] == scanset[i]) {
            break;
         }
      }

      graft_tree_s(t, NODEKIND_DATA, pctx->tok);
   }

   pctx->etok = pctx->tok;
}

static void resync(parse_ctx_t *pctx, const char *follow) {
   pctx->reason = "resync failed";
   skiptoks(pctx, follow);
}

static inline void rewind_tokstate(parse_ctx_t *pctx, size_t orig_idx) {
   pctx->idx = orig_idx;
   pctx->tok = array_peek(pctx->toks, pctx->idx);
}

static inline void check_eoe(parse_ctx_t *pctx) {
   if (pctx->idx == pctx->len - 1) {
      longjmp(pctx->env, NODEKIND__FINALE);
   }
}

static void regerr(parse_ctx_t *pctx) {
   synerr_t err;

   err.etok = pctx->etok;
   err.reason = pctx->reason;

   array_append(pctx->errs, &err, sizeof err);

   pctx->errcnt++;

   if (pctx->errcnt == pctx->errcnt_max) {
      longjmp(pctx->env, NODEKIND__FINALE);
   }
}

static void print_syntax_error(void *item, size_t idx, void *ctx) {
   synerr_t *err;
   parse_ctx_t *pctx;
   size_t lnum, lpos;
   line_t *l;

   err = item;
   pctx = ctx;

   lnum = err->etok->lnum;
   lpos = err->etok->lpos;
   l = array_peek(pctx->ls, lnum - 1);

   safe_vprintf(
      Cbred "\n<syntax error #%zu>" Creset " %s\n"
      "[%s:%zu:%zu] " Cbwhite "note:" Creset " problematic since here\n"
      "%4d|%.*s" Cbblue "%s" Creset "\n",
      idx, err->reason,
      pctx->ov->src, lnum, lpos,
      lnum, lpos - 1, l->run, &l->run[lpos - 1]
   );
}

static void synwarn(parse_ctx_t *pctx) {
   size_t lnum, lpos;
   line_t *l;

   lnum = pctx->etok->lnum;
   lpos = pctx->etok->lpos;
   l = array_peek(pctx->ls, lnum - 1);

   safe_vprintf(
      Cyellow "\n<syntax warning>" Creset " %s\n"
      "[%s:%zu:%zu] " Cbwhite "note:" Creset " problematic since here\n"
      "%4d|%.*s" Cbblue "%s" Creset "\n",
      pctx->reason,
      pctx->ov->src, lnum, lpos,
      lnum, lpos - 1, l->run, &l->run[lpos - 1]
   );
}

static void synerr(parse_ctx_t *pctx) {
   size_t lnum, lpos;
   line_t *l;

   lnum = pctx->etok->lnum;
   lpos = pctx->etok->lpos;
   l = array_peek(pctx->ls, lnum - 1);

   safe_vprintf(
      Cbred "\n<syntax error>" Creset " %s\n"
      "[%s:%zu:%zu] " Cbwhite "note:" Creset " problematic since here\n"
      "%4d|%.*s" Cbblue "%s" Creset "\n",
      pctx->reason,
      pctx->ov->src, lnum, lpos,
      lnum, lpos - 1, l->run, &l->run[lpos - 1]
   );
   exit(EXIT_FAILURE);
}

static tree_t *plant_tree(
   const char *run,
   size_t len,
   nodekind_t kind,
   size_t lnum,
   size_t lpos
) {
   node_t node;
   char *buf;

   if (run) {
      buf = safe_malloc(len + 1);  /* +1 for \0 */
      memcpy(buf, run, len + 1);  /* \0 copied */
   }
   else buf = NULL;

   node.datkind = DATKIND_STR;
   node.dat.s.run = buf;
   node.dat.s.len = len;
   node.kind = kind;
   node.lnum = lnum;
   node.lpos = lpos;

   return tree_plant(&node, sizeof node);
}

static tree_t *graft_tree_s(
   tree_t *base,
   nodekind_t kind,
   const token_t *tok
) {
   tree_t *sub = plant_tree(
      tok->run,
      tok->len,
      kind,
      tok->lnum,
      tok->lpos
   );

   return tree_graft(base, sub);
}

static tree_t *graft_tree_n(
   tree_t *base,
   int val,
   nodekind_t kind,
   const token_t *tok
) {
   node_t node;
   tree_t *sub;

   node.datkind = DATKIND_INT;
   node.dat.n = val;
   node.kind = kind;
   node.lnum = tok->lnum;
   node.lpos = tok->lpos;
   sub = tree_plant(&node, sizeof node);

   return tree_graft(base, sub);
}

static size_t count_tree_node(tree_t *root) {
   size_t cnt, clen;

   cnt = 0;
   clen = tree_clen(root);

   if (clen == 0)
      return 1;

   for (size_t i = 0; i < clen; i++)
      cnt += count_tree_node(tree_child(root, i));

   return cnt + 1;
}

/*** DEBUG ***/

extern void showpt(compile_ctx_t *cctx) {
   tree_pre_traverse(cctx->pt, shownode, 0, NULL);
}

static void shownode(tree_t *t, int lv, void *ctx) {
   static char buf[128];

   node_t *n;
   int cnt, total;

   (void) ctx;
   n = tree_dat(t);
   cnt = sprintf(buf, "%d", lv);
   buf[cnt] = '\0';
   total = lv * strlen("  ");
   total -= cnt;

   fputs(buf, stdout);
   for (int i = 0; i < total; i++)
      putchar(' ');

   printf("[%s] = [", resolve_nodekind(n->kind));
   if (n->datkind == DATKIND_INT)
      printf("%d]\n", n->dat.n);
   else {
      printf("%s]",
         n->dat.s.len
         ? (char *) n->dat.s.run
         : Cbblack "(empty)" Creset
      );
      printf(" " Cbblack "(len = %d)" Creset "\n", n->dat.s.len);
   }
}

static const char *resolve_nodekind(nodekind_t kind) {
   switch (kind) {
      case NODEKIND_ROOT   : return "ROOT";
      case NODEKIND_DATA   : return "DATA";
      case NODEKIND_ROMNUM : return "ROMAN_NUMERAL";
      case NODEKIND_SUBJ   : return "SUBJECT"; // fixme: in use?
      case NODEKIND_CONST  : return "CONST";
      case NODEKIND_ADJ    : return "ADJ";
      case NODEKIND_ZERO   : return "NOUN_ZERO";
      case NODEKIND_PNOUN  : return "NOUN_POS";
      case NODEKIND_NNOUN  : return "NOUN_NEG";
      case NODEKIND_AFFIRM : return "AFFIRM";
      case NODEKIND_NEGATE : return "NEGATE";
      case NODEKIND_CONSEQ : return "CONSEQUENT";
      case NODEKIND_LHS    : return "LHS";
      case NODEKIND_RHS    : return "RHS";
      case NODEKIND_P1     : return "PERSON_1ST";
      case NODEKIND_P2     : return "PERSON_2ND";
      case NODEKIND_P3     : return "PERSON_3RD";
      case NODEKIND_EQ     : return "EQ";
      case NODEKIND_INEQ   : return "INEQ";
      case NODEKIND_GT     : return "INEQ_GT";
      case NODEKIND_LT     : return "INEQ_LT";
      case NODEKIND_TITLE  : return "TITLE";
      case NODEKIND_DP     : return "DP";
      case NODEKIND_CHDECL : return "CHARDECL";
      case NODEKIND_NRTV   : return "NARRATIVE";
      case NODEKIND_CHAR   : return "CHAR";
      case NODEKIND_ACT    : return "ACT";
      case NODEKIND_SCENE  : return "SCENE";
      case NODEKIND_ENTER  : return "ENTER";
      case NODEKIND_EXIT   : return "EXIT";
      case NODEKIND_EXEUNT : return "EXEUNT";
      case NODEKIND_LINE   : return "LINE";
      case NODEKIND_ASGN1  : return "ASSIGN_A";
      case NODEKIND_ASGN2  : return "ASSIGN_B";
      case NODEKIND_ASGN3  : return "ASSIGN_C";
      case NODEKIND_OUT_N  : return "OUT_N";
      case NODEKIND_OUT_C  : return "OUT_C";
      case NODEKIND_IN_N   : return "IN_N";
      case NODEKIND_IN_C   : return "IN_C";
      case NODEKIND_GOTO   : return "GOTO";
      case NODEKIND_COND   : return "COND";
      case NODEKIND_IF     : return "IF";
      case NODEKIND_PUSH   : return "PUSH";
      case NODEKIND_POP    : return "POP";
      case NODEKIND_SUM    : return "SUM";
      case NODEKIND_DIFF   : return "DIFF";
      case NODEKIND_PROD   : return "PROD";
      case NODEKIND_QUOT   : return "QUOT";
      case NODEKIND_REM    : return "REM";
      case NODEKIND_SQRT   : return "SQRT";
      case NODEKIND_SQUR   : return "SQUR";
      case NODEKIND_CUBE   : return "CUBE";
      case NODEKIND_2X     : return "2X";
      case NODEKIND_FACT   : return "FACT";
      default: return NULL;   /* unreachable */
   }
}
