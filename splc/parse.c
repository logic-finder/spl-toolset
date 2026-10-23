#include "parse.h"
#include "parse.internals.h"

// fixme: STREQL 매크로 정의해서 쓰기 (!strcmp 대체 프로젝트 전체적으로)
// fixme: error code를 전파하는 방식으로 변경 (gettok 제외)

extern void parse(compile_ctx_t *cctx) {
   parse_ctx_t pctx = {0};
   int ret;

   /* Initializes pctx struct */
   pctx.arena = cctx->arena;
   pctx.cfg = cctx->cfg;
   pctx.sourcefile = cctx->sourcefile;
   pctx.ls = cctx->ls;
   pctx.toks = cctx->toks;
   pctx.len = array_size(pctx.toks);
   pctx.idx = 0;
   /* Note: toks has at least one element, otherwise the program
      would have been terminated already in loadfile() by splc.c */
   /* Note: didn't use gettok() on purpose since it advances
      pctx.idx, causing an unmatch between current token and its idx */
   pctx.tok = array_peek(pctx.toks, pctx.idx);
   pctx.etok = pctx.tok;
   pctx.reason = "none";
   pctx.pt = plant_tree(&pctx, NULL, 0, NODEKIND_ROOT, 0, 0);
   pctx.errcnt = 0;
   pctx.errcnt_max = 50;
   pctx.errs = array_create(NULL);
   pctx.recovery_point = NODEKIND__UNKNOWN;

   if (cctx->cfg->vbs) safe_fputs(stdout, ENPREFIX "parsing...");

   /* Constructs the parse tree
         pt[0] = title
         pt[1] = dp
         pt[2] = nrtv  */

   /* Note: errcnt is declared volatile in order to prevent
      its value from being restored to the snapshotted value
      after longjmp */
   ret = setjmp(pctx.env_parse);
   if (ret == NODEKIND__FINALE) {
      goto report_error;
   }

   parse_title(&pctx);
   parse_dp(&pctx);
   pctx.nrtv = graft_tree_n(&pctx, pctx.pt, 0, NODEKIND_NRTV, pctx.tok);
   for (;;) {
      parse_act(&pctx);
      for (;;) {
         parse_scene(&pctx);
         for (;;) {
            switch (seek_direction(&pctx)) {
               case NODEKIND_LINE   : parse_line  (&pctx); break;
               case NODEKIND_ENTER  : parse_enter (&pctx); break;
               case NODEKIND_EXIT   : parse_exit  (&pctx); break;
               case NODEKIND_EXEUNT : parse_exeunt(&pctx); break;
               case NODEKIND__FINALE : goto EOE;
               case NODEKIND_ACT     : goto EOA;
               case NODEKIND_SCENE   : goto EOS;
               case NODEKIND__UNKNOWN : break;  /* only after resync */
               default: ;  /* control never reaches default */
            }
         }
      EOS: ;
      }
   EOA: ;
   }
EOE: ;

   /* Reports collected syntax errors, if any */
report_error:
   if (pctx.errcnt > 0) {
      report_syntax_errors(&pctx);
      exit(EXIT_FAILURE);
   }

   if (cctx->cfg->vbs) safe_vprintf(" " Cgreen "done!" Creset
      "\t(total " Cbwhite "%zu" Creset " nodes)\n",
      count_tree_node(pctx.pt)
   );

   /* cleanup */
   array_destroy(pctx.errs);

   /* done! */
   cctx->pt = pctx.pt;
   return;
}

[[deprecated]]
extern void destroy_pt(compile_ctx_t *cctx) {
   tree_post_traverse(cctx->pt, cleanup_node, 0, cctx);
   tree_prune(cctx->pt);
}

static bool cleanup_node(tree_t *t, size_t lv, void *ctx) {
   node_t *n;
   compile_ctx_t *cctx;

   (void) lv;

   n = tree_dat(t);
   cctx = ctx;

   if (n->datkind != DATKIND_STR) {
      return true;
   }

   arena_free_fallback(cctx->arena, n->dat.s.run);

   return true;
}

static nodekind_t seek_direction(parse_ctx_t *pctx) {
   typedef struct mapper {
      seeker_t *seek;
      nodekind_t kind;
   } mapper_t;

   static const mapper_t maps[] = {
      { seek_line   , NODEKIND_LINE   },
      { seek_enter  , NODEKIND_ENTER  },
      { seek_exit   , NODEKIND_EXIT   },
      { seek_scene  , NODEKIND_SCENE  },
      { seek_act    , NODEKIND_ACT    },
      { seek_exeunt , NODEKIND_EXEUNT }
   };
   static const size_t maps_len = ARRLEN(maps);
   const mapper_t *map;

   /* has recovery point been set? */
   if (pctx->recovery_point != NODEKIND__UNKNOWN) {
      nodekind_t temp = pctx->recovery_point;
      pctx->recovery_point = NODEKIND__UNKNOWN;  /* reset */
      return temp;
   }

   /* end of token stream? */
   if (pctx->idx == pctx->len - 1) {
      return NODEKIND__FINALE;
   }

   pctx->reason = msgs.err.syn.eot;
   gettok(pctx);

   /* Since backtracking can happen, we need to save the
      current parsing state */
   const size_t orig_idx = pctx->idx;

   for (size_t i = 0; i < maps_len; i++) {
      map = &maps[i];
      if ((*map->seek)(pctx)) {
         return map->kind;
      }
      rewind_tokstate(pctx, orig_idx);
   }

   pctx->reason = msgs.err.syn.incomprehensible;
   regerr(pctx);
   /* the current tok will get the same check once again, but let's go easy */
   resyncbf_all(pctx);
   return NODEKIND__UNKNOWN;
}

static void report_syntax_errors(parse_ctx_t *pctx) {
   safe_fputc(stdout, '\n');
   array_foreach(pctx->errs, print_syntax_error, pctx);

   if (pctx->errcnt < pctx->errcnt_max) {
      return;
   }

   safe_vprintf("too many syntax errors."
      " Aborting parsing at [%s:%zu:%zu]\n",
      pctx->sourcefile, pctx->etok->lnum, pctx->etok->lpos
   );
}

static void parse_title(parse_ctx_t *pctx) {
   tree_t *title;

   title = graft_tree_n(pctx, pctx->pt, 0, NODEKIND_TITLE, pctx->tok);

   graft_tree_s(pctx, title, NODEKIND_DATA, pctx->tok);

   pctx->reason = msgs.err.syn.title.incomp;
   readtoks_until(pctx, end_symbols, title);
}

static void parse_dp(parse_ctx_t *pctx) {
   tree_t *dp, *character;

   dp = graft_tree_n(pctx, pctx->pt, 0, NODEKIND_DP, pctx->tok);

   for (;;) {
      gettok(pctx);
      character = graft_tree_n(pctx, dp, 0, NODEKIND_CHDECL, pctx->tok);

      pctx->reason = msgs.err.syn.dp.chardecl_incomp;
      readtoks_until(pctx, ",", character);

      if (tree_clen(character) == 0) {
         pctx->reason = msgs.err.syn.dp.noname;
         regerr(pctx);
      }

      pctx->reason = msgs.err.syn.dp.desc_incomp;
      skiptoks(pctx, end_symbols);

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
   if (strcasecmp(pctx->tok->run, KEYWRD_ACT))
      return 0;
   gettokn(pctx, 2);
   if (pctx->tok->run[0] != ':')
      return 0;
   else
      return 1;
}

static void parse_act(parse_ctx_t *pctx) {
   pctx->recovery_point = NODEKIND__UNKNOWN;  /* reset */

   ungettokn(pctx, 2);

   if (pctx->cfg->w_kc && strcmp(pctx->tok->run, KEYWRD_ACT)) {
      pctx->reason = "this keyword should be 'Act'";
      synwarn(pctx);
   }

   pctx->act = graft_tree_n(
      pctx, pctx->nrtv, 0, NODEKIND_ACT, pctx->tok
   );

   pctx->reason = msgs.err.syn.act.incomp;
   gettok(pctx);

   if (!is_rnum(pctx->tok->run)) {
      graft_tree_s(pctx, pctx->act, NODEKIND__ERROR, pctx->tok);
      pctx->reason = msgs.err.syn.act.nornum;
      regerr(pctx);
      resyncbf(pctx, ResyncAct | ResyncScene);
      return;
   }
   else {
      graft_tree_s(pctx, pctx->act, NODEKIND_ROMNUM, pctx->tok);
      gettok(pctx);
   }

   if (pctx->tok->run[0] != ':') {
      pctx->reason = msgs.err.syn.act.nocolon;
      regerr(pctx);
      resyncbf(pctx, ResyncAct | ResyncScene);
      return;
   }

   pctx->reason = msgs.err.syn.act.desc_incomp;
   skiptoks(pctx, end_symbols);

   pctx->reason = msgs.err.syn.act.noscene;
   gettok(pctx);

   if (!seek_scene(pctx)) {
      regerr(pctx);
      resyncbf(pctx, ResyncAct | ResyncScene);
      return;
   }
}

static int seek_scene(parse_ctx_t *pctx) {
   if (strcasecmp(pctx->tok->run, KEYWRD_SCENE))
      return 0;
   gettokn(pctx, 2);
   if (pctx->tok->run[0] != ':')
      return 0;
   else
      return 1;
}

static void parse_scene(parse_ctx_t *pctx) {
   pctx->recovery_point = NODEKIND__UNKNOWN;  /* reset */

   ungettokn(pctx, 2);

   if (pctx->cfg->w_kc && strcmp(pctx->tok->run, KEYWRD_SCENE)) {
      pctx->reason = "this keyword should be 'Scene'";
      synwarn(pctx);
   }

   pctx->scene = graft_tree_n(
      pctx, pctx->act, 0, NODEKIND_SCENE, pctx->tok
   );

   pctx->reason = msgs.err.syn.scene.incomp;
   gettok(pctx);

   if (!is_rnum(pctx->tok->run)) {
      graft_tree_s(pctx, pctx->scene, NODEKIND__ERROR, pctx->tok);
      pctx->reason = msgs.err.syn.scene.nornum;
      regerr(pctx);
      resyncbf(pctx, ResyncAct | ResyncScene);
      return;
   }
   else {
      graft_tree_s(pctx, pctx->scene, NODEKIND_ROMNUM, pctx->tok);
      gettok(pctx);
   }

   if (pctx->tok->run[0] != ':') {
      pctx->reason = msgs.err.syn.scene.nocolon;
      regerr(pctx);
      resyncbf(pctx, ResyncAct | ResyncScene);
      return;
   }

   pctx->reason = msgs.err.syn.scene.desc_incomp;
   skiptoks(pctx, end_symbols);
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

   if (pctx->cfg->w_kc && strcmp(pctx->tok->run, KEYWRD_ENTER)) {
      pctx->reason = "this keyword should be 'Enter'";
      synwarn(pctx);
   }

   enter = graft_tree_n(pctx, pctx->scene, 0, NODEKIND_ENTER, pctx->tok);

   if (!parse_namelist(pctx, enter))
      return;  // fixme: use explicit return value

   if (tree_clen(enter)) {
      return;
   }

   pctx->reason = msgs.err.syn.enter.nochar;
   regerr(pctx);
}

static int seek_exit(parse_ctx_t *pctx) {
   return seek_enterlike(pctx, KEYWRD_EXIT);
}

static void parse_exit(parse_ctx_t *pctx) {
   tree_t *exit;

   if (pctx->cfg->w_kc && strcmp(pctx->tok->run, KEYWRD_EXIT)) {
      pctx->reason = "this keyword should be 'Exit'";
      synwarn(pctx);
   }

   exit = graft_tree_n(pctx, pctx->scene, 0, NODEKIND_EXIT, pctx->tok);

   if (!parse_namelist(pctx, exit))
      return;  // fixme: use explicit return value

   switch (tree_clen(exit)) {
      case 0  : pctx->reason = msgs.err.syn.exit.nochar; break;
      case 1  : return;
      default : pctx->reason = msgs.err.syn.exit.exceed;
   }
   regerr(pctx);
}

static int seek_exeunt(parse_ctx_t *pctx) {
   return seek_enterlike(pctx, KEYWRD_EXEUNT);
}

static void parse_exeunt(parse_ctx_t *pctx) {
   tree_t *exeunt;

   if (pctx->cfg->w_kc && strcmp(pctx->tok->run, KEYWRD_EXEUNT)) {
      pctx->reason = "this keyword should be 'Exeunt'";
      synwarn(pctx);
   }

   exeunt = graft_tree_n(pctx, pctx->scene, 0, NODEKIND_EXEUNT, pctx->tok);

   if (!parse_namelist(pctx, exeunt))
      return;  // fixme: use explicit return value

   switch (tree_clen(exeunt)) {
      case 0  : return;
      case 1  : pctx->reason = msgs.err.syn.exeunt.onechar; break;
      case 2  : return;
      default : pctx->reason = msgs.err.syn.exeunt.exceed;
   }
   regerr(pctx);
}

static int seek_line(parse_ctx_t *pctx) {
   if (is_name(pctx) == false) {
      return 0;
   }

   gettok(pctx);

   if (pctx->tok->run[0] != ':') {
      return 0;
   }

   return 1;
}

static void parse_line(parse_ctx_t *pctx) {
   /* Note: charidx has been updated by is_name() in seek_line() */
   pctx->line = graft_tree_n(pctx, pctx->scene, 0, NODEKIND_LINE, pctx->tok);
   graft_tree_n(pctx, pctx->line, pctx->charidx, NODEKIND_CHAR, pctx->tok);

   pctx->reason = msgs.err.syn.line.incomp;

   for (bool found;;) {
      gettok(pctx);
      found = parse_line_router(pctx, stmt_hdlrs, ARRLEN(stmt_hdlrs));
      if (!found) {
         ungettok(pctx);
         return;  /* control returns to parse() */
      }
   }
}

static bool parse_line_router(
   parse_ctx_t *pctx,
   const stmthandler_t *table,
   size_t tsiz
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

   return (i == tsiz) ? false : true;
}

static bool parse_line_as_conseq(parse_ctx_t *pctx) {
   const stmthandler_t *table = stmt_hdlrs;
   size_t tsiz = ARRLEN(stmt_hdlrs);

   pctx->reason = msgs.err.syn.ifstmt.conseq_incomp;
   gettok(pctx);

   // TODO: 대소문자 구별 필요없을듯? -Wkeyword-case 대응
   if (isupper(pctx->tok->run[0])) {
      pctx->reason = msgs.err.syn.ifstmt.conseq_cap;
      regerr(pctx);
   }
   else
   if (islower(pctx->tok->run[0])) {
      pctx->tok->run[0] = toupper(pctx->tok->run[0]);
   }

   /* the standard says 'if' statement can't have an 'if' statement as a consequent */
   if (pctx->cfg->std == StdoptSpl01) {
      table = &stmt_hdlrs[1];
      tsiz--;
   }

   return parse_line_router(pctx, table, tsiz);
}

static int seek_if(parse_ctx_t *pctx) {
   return strcasecmp(pctx->tok->run, KEYWRD_IF) ? 0 : 1;
}

static void parse_if(parse_ctx_t *pctx) {
   tree_t *ifstmt, *consequent, *temp_line;

   ifstmt = graft_tree_n(pctx, pctx->line, 0, NODEKIND_IF, pctx->tok);

   pctx->reason = msgs.err.syn.ifstmt.incomp;
   gettok(pctx);

   if (!strcasecmp(pctx->tok->run, KEYWRD_SO)) {
      graft_tree_n(pctx, ifstmt, 0, NODEKIND_AFFIRM, pctx->tok);
   }
   else
   if (!strcasecmp(pctx->tok->run, KEYWRD_NOT)) {
      graft_tree_n(pctx, ifstmt, 0, NODEKIND_NEGATE, pctx->tok);
   }
   else {
      pctx->reason = msgs.err.syn.ifstmt.badsyn;
      regerr(pctx);
      resyncbf_all(pctx);
      return;
   }

   gettok(pctx);

   if (pctx->tok->run[0] != ',') {
      pctx->reason = msgs.err.syn.ifstmt.badsyn;
      regerr(pctx);
      resyncbf_all(pctx);
      return;
   }

   /* Parses the consequent */
   consequent = graft_tree_n(pctx, ifstmt, 0, NODEKIND_CONSEQ, pctx->tok);
   temp_line = pctx->line;
   pctx->line = consequent;

   if (!parse_line_as_conseq(pctx)) {
      pctx->reason = msgs.err.syn.ifstmt.bad_conseq;
      regerr(pctx);
      resyncbf_all(pctx);
      return;
   }

   pctx->line = temp_line;
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
         if (!asgn) return;  /* parse error */
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

   if (!parse_const(pctx, asgn))
      return;  // fixme: after handle check_const_end first

   if (strchr(stmt_symbols, pctx->tok->run[0])) {
      return;
   }

   pctx->reason = msgs.err.syn.asgn.invalid_end_symbol;
   regerr(pctx);
   resyncbf_all(pctx);
}

static tree_t *parse_asgn_i(parse_ctx_t *pctx, token_t *you) {
   /* TYPE 1: You A(.|!) */
   return graft_tree_s(pctx, pctx->line, NODEKIND_ASGN1, you);
}

static tree_t *parse_asgn_ii(parse_ctx_t *pctx, token_t *you) {
   /* TYPE 2: You be as adj as (B|C|D)(.|!) */

   if (pctx->cfg->w_bp) {
      check_asgn_predicate(pctx);
   }

   pctx->reason = msgs.err.syn.asgn.incomp;
   gettok(pctx);

   if (!query_adj_lower(pctx->tok->run)) {
      pctx->reason = msgs.err.syn.asgn.not_adj;
      regerr(pctx);
      return NULL;
   }

   pctx->reason = msgs.err.syn.asgn.incomp;
   gettok(pctx);

   if (strcasecmp(pctx->tok->run, KEYWRD_AS)) {
      pctx->reason = msgs.err.syn.asgn.not_as;
      regerr(pctx);
      return NULL;
   }

   return graft_tree_s(pctx, pctx->line, NODEKIND_ASGN2, you);
}

static tree_t *parse_asgn_iii(parse_ctx_t *pctx, token_t *you) {
   /* TYPE 3: You be (B|C|D)(.|!) */

   if (pctx->cfg->w_bp) {
      check_asgn_predicate(pctx);
   }

   return graft_tree_s(pctx, pctx->line, NODEKIND_ASGN3, you);
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

   graft_tree_n(pctx, pctx->line, 0, kind, pctx->tok);

   pctx->reason = msgs.err.syn.out.incomp;
   gettok(pctx);

   test1 = !strcasecmp(pctx->tok->run, KEYWRD_YOUR)
      || !strcasecmp(pctx->tok->run, KEYWRD_THINE)
      || !strcasecmp(pctx->tok->run, KEYWRD_THY);

   if (!test1) {
      pctx->reason = msgs.err.syn.out.badsyn;
      regerr(pctx);
      resyncbf_all(pctx);
      return;
   }

   gettok(pctx);

   test2 = (kind == NODEKIND_OUT_N) && (!strcasecmp(pctx->tok->run, KEYWRD_MIND));
   test3 = (kind == NODEKIND_OUT_C) && (!strcasecmp(pctx->tok->run, KEYWRD_HEART));

   if (test2 || test3) {
      pctx->reason = msgs.err.syn.out.unmatched;
      regerr(pctx);
      resyncbf_all(pctx);
      return;
   }

   gettok(pctx);

   if (!match(pctx->tok->run[0], stmt_symbols)) {
      pctx->reason = msgs.err.syn.out.badsyn;
      regerr(pctx);
      resyncbf_all(pctx);
      return;
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

   graft_tree_n(pctx, pctx->line, 0, kind, pctx->tok);
   pctx->reason = msgs.err.syn.in.incomp;

   if (kind == NODEKIND_IN_N) {
      gettok(pctx);
      if (strcasecmp(pctx->tok->run, KEYWRD_TO)) {
         pctx->reason = msgs.err.syn.in.badsyn;
         regerr(pctx);
         resyncbf_all(pctx);
         return;
      }
   }

   gettok(pctx);

   test1 = !strcasecmp(pctx->tok->run, KEYWRD_YOUR)
      || !strcasecmp(pctx->tok->run, KEYWRD_THINE)
      || !strcasecmp(pctx->tok->run, KEYWRD_THY);

   if (!test1) {
      pctx->reason = msgs.err.syn.in.badsyn;
      regerr(pctx);
      resyncbf_all(pctx);
      return;
   }

   gettok(pctx);

   test2 = (kind == NODEKIND_OUT_N) && (!strcasecmp(pctx->tok->run, KEYWRD_MIND));
   test3 = (kind == NODEKIND_OUT_C) && (!strcasecmp(pctx->tok->run, KEYWRD_HEART));

   if (test2 || test3) {
      pctx->reason = msgs.err.syn.in.unmatched;
      regerr(pctx);
      resyncbf_all(pctx);
      return;
   }

   gettok(pctx);

   if (!match(pctx->tok->run[0], stmt_symbols)) {
      pctx->reason = msgs.err.syn.in.badsyn;
      regerr(pctx);
      resyncbf_all(pctx);
      return;
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
      regerr(pctx);
      resyncbf_all(pctx);
      return;
   }

   test1 = (type == GototypeWe) && test1;
   test2 = (type == GototypeLet) && test2;

   if (test1 || test2) {
      pctx->reason = msgs.err.syn.gt.unmatched;
      regerr(pctx);
      resyncbf_all(pctx);
      return;
   }

   gettok(pctx);

   test3 = strcasecmp(pctx->tok->run, KEYWRD_RETURN);
   test4 = strcasecmp(pctx->tok->run, KEYWRD_PROCED);

   if (test3 && test4) {
      pctx->reason = msgs.err.syn.gt.badsyn;
      regerr(pctx);
      resyncbf_all(pctx);
      return;
   }

   gettok(pctx);

   if (strcasecmp(pctx->tok->run, KEYWRD_TO)) {
      pctx->reason = msgs.err.syn.gt.badsyn;
      regerr(pctx);
      resyncbf_all(pctx);
      return;
   }

   gettok(pctx);

   // TODO: support -Wkeyword-case
   if (!strcasecmp(pctx->tok->run, KEYWRD_ACT)) {
      dest = NODEKIND_ACT;
   }
   else
   if (!strcasecmp(pctx->tok->run, KEYWRD_SCENE)) {
      dest = NODEKIND_SCENE;
   }
   else {
      pctx->reason = msgs.err.syn.gt.badsyn;
      regerr(pctx);
      resyncbf_all(pctx);
      return;
   }

   gt = graft_tree_n(pctx, pctx->line, dest, NODEKIND_GOTO, pctx->tok);

   gettok(pctx);

   // fixme: here
   // if (is_romnum(pctx->tok->run) == false) {
   if (0) {
      pctx->reason = msgs.err.syn.gt.badsyn;
      regerr(pctx);
      resyncbf_all(pctx);
      return;
   }

   graft_tree_s(pctx, gt, NODEKIND_ROMNUM, pctx->tok);

   gettok(pctx);

   if (!match(pctx->tok->run[0], stmt_symbols)) {
      pctx->reason = msgs.err.syn.gt.badsyn;
      regerr(pctx);
      resyncbf_all(pctx);
      return;
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

   condition = graft_tree_n(pctx, pctx->line, 0, NODEKIND_COND, pctx->tok);
   lefthand  = graft_tree_n(pctx, condition, 0, NODEKIND_LHS, pctx->tok);

   pctx->reason = msgs.err.syn.cond.incomp;
   gettok(pctx);

   /* Am [I] not better than yourself?
      Art [thou] not more cunning than the Ghost?
      Is [a] tree not as good as a shiny tree? */

   /* checks the relation between the subject and the be verb conjugation
      only if --Wbe-predicate is activated */
   if (pctx->cfg->w_bp) {
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

   p = graft_tree_n(pctx, lefthand, 0, kind, pctx->tok);

   if (pctx->be_kind == 3) {
      /* parse_const() has consumed "a tree" */
      if (!parse_const(pctx, p))
         return; // fixme: after handle check_const_end first
      /* Is a tree [not] as good as a shiny tree? */
   } else {
      pctx->reason = msgs.err.syn.cond.incomp;
      gettok(pctx);
      /* Am I [not] better than yourself?
         Art thou [not] more cunning than the Ghost? */
   }

   if (!strcasecmp(pctx->tok->run, KEYWRD_NOT)) {
      graft_tree_n(pctx, condition, 0, NODEKIND_NEGATE, pctx->tok);
      pctx->reason = msgs.err.syn.cond.incomp;
      gettok(pctx);
      /* Am I not [better] than yourself?
         Art thou not [more] cunning than the Ghost?
         Is a tree not [as] good as a shiny tree? */
   }
   else
      graft_tree_n(pctx, condition, 0, NODEKIND_AFFIRM, pctx->tok);

   if (!strcasecmp(pctx->tok->run, KEYWRD_AS)) {
      if (!parse_cond_eq(pctx, condition))
         return;  // fixme: use explicit return value
      /* Is a tree not as good [as] a shiny tree? */
   }
   else {
      if (!parse_cond_ineq(pctx, condition))
         return;  // fixme: use explicit return value
      /* Am I not better [than] yourself?
         Art thou not more cunning [than] the Ghost? */
   }

   righthand = graft_tree_n(pctx, condition, 0, NODEKIND_RHS, pctx->tok);
   /* The last noun is to be comsumed */
   if (!parse_const(pctx, righthand))
      return;  // fixme: after handle check_const_end first
   /* Am I not better than yourself[?]
      Art thou not more cunning than the Ghost[?]
      Is a tree not as good as a shiny tree[?] */
}

static int parse_cond_eq(parse_ctx_t *pctx, tree_t *cond) {
   /* Is a tree not [as] good as a shiny tree? */
   gettok(pctx);
   /* Is a tree not as [good] as a shiny tree? */

   if (!query_adj_lower(pctx->tok->run)) {
      pctx->reason = msgs.err.syn.cond.badsyn;
      regerr(pctx);
      resyncbf_all(pctx);
      return 0;
   }

   graft_tree_s(pctx, cond, NODEKIND_EQ, pctx->tok);

   gettok(pctx);
   /* Is a tree not as good [as] a shiny tree? */

   if (strcasecmp(pctx->tok->run, KEYWRD_AS)) {
      pctx->reason = msgs.err.syn.cond.badsyn;
      regerr(pctx);
      resyncbf_all(pctx);
      return 0;
   }

   return 1;
}

static int parse_cond_ineq(parse_ctx_t *pctx, tree_t *cond) {
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

      if (!query_adj_lower(pctx->tok->run)) {
         pctx->reason = msgs.err.syn.cond.badsyn;
         regerr(pctx);
         resyncbf_all(pctx);
         return 0;
      }

      switch (ret) {
         case 0 : kind = NODEKIND_GT; break;
         case 1 : kind = NODEKIND_LT; break;
         default: ;  /* control never reaches default */
      }

      graft_tree_s(pctx, cond, kind, pctx->tok);
   }
   /* one word comparative like "better" or "worse" */
   else {
      graft_tree_s(pctx, cond, NODEKIND_INEQ, pctx->tok);
   }

   pctx->reason = msgs.err.syn.cond.incomp;
   gettok(pctx);
   /* Am I not better [than] yourself?
      Art thou not more cunning [than] the Ghost? */

   if (strcasecmp(pctx->tok->run, KEYWRD_THAN)) {
      pctx->reason = msgs.err.syn.cond.badsyn;
      regerr(pctx);
      resyncbf_all(pctx);
      return 0;
   }

   return 1;
}

static int seek_push(parse_ctx_t *pctx) {
   return strcasecmp(pctx->tok->run, KEYWRD_REMEMB) ? 0 : 1;
}

static void parse_push(parse_ctx_t *pctx) {
   tree_t *push;

   push = graft_tree_n(pctx, pctx->line, 0, NODEKIND_PUSH, pctx->tok);

   if (!parse_const(pctx, push))
      return;  // fixme: after handle check_const_end first

   if (!match(pctx->tok->run[0], stmt_symbols)) {
      pctx->reason = msgs.err.syn.push.badsyn;
      regerr(pctx);
      resyncbf_all(pctx);
      return;
   }
}

static int seek_pop(parse_ctx_t *pctx) {
   return strcasecmp(pctx->tok->run, KEYWRD_RECALL) ? 0 : 1;
}

static void parse_pop(parse_ctx_t *pctx) {
   graft_tree_n(pctx, pctx->line, 0, NODEKIND_POP, pctx->tok);
   pctx->reason = msgs.err.syn.pop.incomp;
   skiptoks(pctx, end_symbols);
}

// TODO: 파싱 결과가 2명 이상인데 and가 없었을 경우 오류
// TODO: and가 2번 이상 등장해도 오류
// TODO: 중복된 이름이 있는지 같은것은 ctxcheck에서 하기
static int parse_namelist(parse_ctx_t *pctx, tree_t *t) {
   /* Enter, Exit, and Exeunt takes a namelist:
         (1) [<enterlike> A]
         (2) [<enterlike> A and B]
         (3) [<enterlike> A, B, and C] */
   const char *follow_set[] = {
      "]", "and", ",", NULL
   };

   pctx->reason = "incomplete namelist";
   gettok(pctx);
   if (pctx->tok->run[0] == ']')
      return 1;
   ungettok(pctx);

   /* Currently, tok->run points to <enterlike> */

   /* Consumes tokens until "and" */
   do {
      pctx->reason = "incomplete namelist"; // fixme: remove this
      gettok(pctx);
      if (!is_name_lower(pctx)) {
         pctx->reason = "dp expected here";
         regerr(pctx);
         resyncbf_all(pctx);
         // fixme: remove this
         // resyncfs(pctx, follow_set);
         // ungettok(pctx);
         return 0;
      }
      graft_tree_n(pctx, t, pctx->charidx, NODEKIND_CHAR, pctx->tok);
      gettok(pctx);
      if (pctx->tok->run[0] == ']')
         return 1;
      if (!strcasecmp(pctx->tok->run, KEYWRD_AND))
         break;
      if (pctx->tok->run[0] != ',') {
         pctx->reason = ", expected here";
         regerr(pctx);
         resyncbf_all(pctx);
         return 0;
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
      regerr(pctx);
      resyncbf_all(pctx);
      // fixme: remove this
      // resync(pctx, "]");
      // ungettok(pctx);
      return 0;
   }
   graft_tree_n(pctx, t, pctx->charidx, NODEKIND_CHAR, pctx->tok);
   gettok(pctx);
   if (pctx->tok->run[0] != ']') {
      pctx->reason = "] expected here";
      regerr(pctx);
      resyncbf_all(pctx);
      // fixme: remove this
      // resync(pctx, "]");
      return 0;
   }

   return 1;
}

static int parse_const(parse_ctx_t *pctx, tree_t *stmt) {
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
   cnst = graft_tree_n(pctx, stmt, 0, NODEKIND_CONST, pctx->tok);

   pctx->reason = msgs.err.syn.cnst.incomp;
   gettok(pctx);

   /* First of all, we check whether this is an operator */
   if ((kind = seek_op(pctx)) != NODEKIND__NAO) {
      return parse_op(pctx, cnst, kind);
   }

   /* Checks whether this token is
      a pronoun, a reflexive, a name, or a nil */
   if (is_pronoun(pctx->tok->run)) {
      graft_tree_s(pctx, cnst, what_pronoun(pctx->tok->run), pctx->tok);
      if (!check_const_end(pctx)) return 0;  // fixme: use explicit enum
      return 1;
   }
   if (is_reflexive(pctx->tok->run)) {
      graft_tree_s(pctx, cnst, what_reflexive(pctx->tok->run), pctx->tok);
      if (!check_const_end(pctx)) return 0;  // fixme: use explicit enum
      return 1;
   }
   if (is_name_lower(pctx)) {
      graft_tree_n(pctx, cnst, pctx->charidx, NODEKIND_CHAR, pctx->tok);
      if (!check_const_end(pctx)) return 0;  // fixme: use explicit enum
      return 1;
   }
   if (is_nil(pctx->tok->run)) {
      graft_tree_s(pctx, cnst, NODEKIND_ZERO, pctx->tok);
      if (!check_const_end(pctx)) return 0;  // fixme: use explicit enum
      return 1;
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
      graft_tree_s(pctx, cnst, NODEKIND_ADJ, pctx->tok);
      gettok(pctx);
   }

   /* noun */
   if (query_noun(pctx->tok->run, &query_result)) {
      kind = query_result ? NODEKIND_PNOUN : NODEKIND_NNOUN;
      graft_tree_s(pctx, cnst, kind, pctx->tok);
   }
   /* noun phrase */
   else {
      if (!parse_noun_phrase(pctx, cnst))
         return 0;  // fixme: use explicit return value
   }

   /* end of const */
   if (!check_const_end(pctx)) return 0;  // fixme: use explicit enum

   return 1;
}

static int parse_noun_phrase(parse_ctx_t *pctx, tree_t *cnst) {
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

   // TODO: it'd be nice to have arena_strcat with separator
   bufsiz = prev_tok->len + 1 + pctx->tok->len;
   buf = arena_malloc_fallback(pctx->arena, bufsiz + 1);  /* +1 for \0 */
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
      regerr(pctx);
      resyncbf_all(pctx);
      return 0;
   }

   kind = query_result ? NODEKIND_PNOUN : NODEKIND_NNOUN;

   tok.run = buf;
   tok.len = bufsiz;
   tok.lnum = pctx->tok->lnum;
   tok.lpos = pctx->tok->lpos;

   graft_tree_s(pctx, cnst, kind, &tok);

   arena_free_fallback(pctx->arena, buf);

   return 1;
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

static int parse_op(parse_ctx_t *pctx, tree_t *stmt, nodekind_t kind) {
   tree_t *op;

   op = graft_tree_n(pctx, stmt, 0, kind, pctx->tok);
   pctx->reason = msgs.err.syn.op.incomp;
   switch (kind) {
      case NODEKIND_SUM  : return parse_op_sum (pctx, op);
      case NODEKIND_DIFF : return parse_op_diff(pctx, op);
      case NODEKIND_PROD : return parse_op_prod(pctx, op);
      case NODEKIND_QUOT : return parse_op_quot(pctx, op);
      case NODEKIND_REM  : return parse_op_rem (pctx, op);
      case NODEKIND_SQRT : return parse_op_sqrt(pctx, op);
      case NODEKIND_SQUR : return parse_op_squr(pctx, op);
      case NODEKIND_CUBE : return parse_op_cube(pctx, op);
      case NODEKIND_2X   : return parse_op_2x  (pctx, op);
      case NODEKIND_FACT : return parse_op_fact(pctx, op);
      // fixme: use dummy value (explicit)
      default: return 2;  /* control never reaches default */
   }
}

static int parse_op_unary(
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
      regerr(pctx);
      resyncbf_all(pctx);
      return 0;
   }

   return parse_const(pctx, op);
}

static int parse_op_binary(
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
      regerr(pctx);
      resyncbf_all(pctx);
      return 0;
   }

   lefthand = graft_tree_n(pctx, op, 0, NODEKIND_LHS, pctx->tok);
   if (!parse_const(pctx, lefthand))
      return 0;  // fixme: after handle check_const_end first

   if (strcasecmp(pctx->tok->run, KEYWRD_AND)) {
      pctx->reason = msgs.err.syn.op.no_and;
      regerr(pctx);
      resyncbf_all(pctx);
      return 0;
   }

   righthand = graft_tree_n(pctx, op, 0, NODEKIND_RHS, pctx->tok);

   return parse_const(pctx, righthand);
}

static inline int parse_op_sum(parse_ctx_t *pctx, tree_t *op) {
   return parse_op_binary(pctx, op, KEYWRD_OF, msgs.err.syn.op.sum);
}

static inline int parse_op_diff(parse_ctx_t *pctx, tree_t *op) {
   return parse_op_binary(pctx, op, KEYWRD_BTW, msgs.err.syn.op.diff);
}

static inline int parse_op_prod(parse_ctx_t *pctx, tree_t *op) {
   return parse_op_binary(pctx, op, KEYWRD_OF, msgs.err.syn.op.prod);
}

static inline int parse_op_quot(parse_ctx_t *pctx, tree_t *op) {
   return parse_op_binary(pctx, op, KEYWRD_BTW, msgs.err.syn.op.quot);
}

static inline int parse_op_rem(parse_ctx_t *pctx, tree_t *op) {
   /* the remainder of
         the quotient between <const> and <const> */

   pctx->reason = msgs.err.syn.op.incomp;
   gettok(pctx);

   if (strcasecmp(pctx->tok->run, KEYWRD_OF)) {
      pctx->reason = msgs.err.syn.op.rem;
      regerr(pctx);
      resyncbf_all(pctx);
      return 0;
   }

   gettok(pctx);

   if (strcasecmp(pctx->tok->run, KEYWRD_THE)) {
      pctx->reason = msgs.err.syn.op.rem_quot_1;
      regerr(pctx);
      return 0;
   }

   gettok(pctx);

   if (strcasecmp(pctx->tok->run, KEYWRD_QUOT)) {
      pctx->reason = msgs.err.syn.op.rem_quot_2;
      regerr(pctx);
      return 0;
   }

   return parse_op_quot(pctx, op);
}

static inline int parse_op_sqrt(parse_ctx_t *pctx, tree_t *op) {
   return parse_op_unary(pctx, op, msgs.err.syn.op.sqrt);
}

static inline int parse_op_squr(parse_ctx_t *pctx, tree_t *op) {
   return parse_op_unary(pctx, op, msgs.err.syn.op.squr);
}

static inline int parse_op_cube(parse_ctx_t *pctx, tree_t *op) {
   return parse_op_unary(pctx, op, msgs.err.syn.op.cube);
}

static inline int parse_op_2x(parse_ctx_t *pctx, tree_t *op) {
   return parse_const(pctx, op);
}

static inline int parse_op_fact(parse_ctx_t *pctx, tree_t *op) {
   return parse_op_unary(pctx, op, msgs.err.syn.op.fact);
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

// TODO: 이 함수의 필요성? 그냥 명사로 끝나거나 하면 상수가 끝난거 아닌가?
static bool check_const_end(parse_ctx_t *pctx) {
   pctx->reason = msgs.err.syn.cnst.incomp;
   gettok(pctx);

   if (strchr(end_symbols, pctx->tok->run[0])
      || !strcasecmp(pctx->tok->run, KEYWRD_NOT)
      || !strcasecmp(pctx->tok->run, KEYWRD_AS)
      || !strcasecmp(pctx->tok->run, KEYWRD_MORE)
      || !strcasecmp(pctx->tok->run, KEYWRD_LESS)
      || !strcasecmp(pctx->tok->run, KEYWRD_AND)
      || query_comp_lower(pctx->tok->run, NULL))
   {
      return true;
   }

   pctx->reason = msgs.err.syn.cnst.no_end_symbol;
   regerr(pctx);
   resyncbf_all(pctx);
   return false;
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
      chsiz = tree_clen(ch);

      if (chsiz == 0) {  /* can be ZERO. see parse_dp() */
         return false;
      }

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

// TODO: support -Wkeyword-case
static bool is_name_lower(parse_ctx_t *pctx) {
   bool modified = false;
   bool parse_result = false;

   if (!strcmp(pctx->tok->run, "A")
      || !strcmp(pctx->tok->run, "An")
      || !strcmp(pctx->tok->run, "The"))
   {
      pctx->reason = msgs.err.syn.name_not_lowcase;
      regerr(pctx);
   }

   if (!strcmp(pctx->tok->run, KEYWRD_A)
      || !strcmp(pctx->tok->run, KEYWRD_AN)
      || !strcmp(pctx->tok->run, KEYWRD_THE))
   {
      modified = true;
      pctx->tok->run[0] = toupper(pctx->tok->run[0]);
   }

   parse_result = is_name(pctx);

   if (modified)
      pctx->tok->run[0] = tolower(pctx->tok->run[0]);

   return parse_result;
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
      longjmp(pctx->env_parse, NODEKIND__FINALE);
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
      longjmp(pctx->env_parse, NODEKIND__FINALE);
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

[[maybe_unused]]
static void eqtok(parse_ctx_t *pctx, char ch) {
   if (pctx->tok->run[0] != ch) {
      return;
   }
   pctx->etok = pctx->tok;
   regerr(pctx);
}

[[maybe_unused]]
static void neqtok(parse_ctx_t *pctx, char ch) {
   if (pctx->tok->run[0] == ch) {
      return;
   }
   pctx->etok = pctx->tok;
   regerr(pctx);
}

// TODO: consider remove this
static void readtoks(parse_ctx_t *pctx, char sentinel, tree_t *base) {
   for (;;) {
      if (pctx->tok->run[0] == sentinel) {
         break;
      }
      graft_tree_s(pctx, base, NODEKIND_DATA, pctx->tok);
      nexttok(pctx);
   }

   pctx->etok = pctx->tok;
}

static void readtoks_until(parse_ctx_t *pctx, const char *scanset, tree_t *t) {
   for (;;) {
      for (size_t i = 0; (scanset[i] != '\0'); i++) {
         if (pctx->tok->run[0] == scanset[i]) {
            goto done;
         }
      }
      graft_tree_s(pctx, t, NODEKIND_DATA, pctx->tok);
      nexttok(pctx);
   }

done:
   pctx->etok = pctx->tok;
}

static void resync(parse_ctx_t *pctx, const char *follow) {
   pctx->reason = "resync failed";
   skiptoks(pctx, follow);
}

static void resyncbf(parse_ctx_t *pctx, resync_bitfield_t bf) {
   typedef struct {
      resync_bitfield_t v;
      seeker_t *seek;
      nodekind_t kind;
   } resync_entry_t;

   static const resync_entry_t entries[] = {
      { ResyncAct    , seek_act    , NODEKIND_ACT    },
      { ResyncScene  , seek_scene  , NODEKIND_SCENE  },
      { ResyncEnter  , seek_enter  , NODEKIND_ENTER  },
      { ResyncExit   , seek_exit   , NODEKIND_EXIT   },
      { ResyncExeunt , seek_exeunt , NODEKIND_EXEUNT },
      { ResyncLine   , seek_line   , NODEKIND_LINE   }
   };
   static const size_t entries_len = ARRLEN(entries);

   const resync_entry_t *entry;

   pctx->reason = "resync failed";

   for (;;) {
      /* Since backtracking can happen, we need to save the
         current parsing state */
      const size_t orig_idx = pctx->idx;

      for (size_t i = 0; i < entries_len; i++) {
         entry = &entries[i];
         if ((entry->v & bf) == false) {
            continue;
         }
         if ((*entry->seek)(pctx)) {
            pctx->recovery_point = entry->kind;
            return;
         }
         rewind_tokstate(pctx, orig_idx);
      }

      gettok(pctx);
   }
}

static inline void resyncbf_all(parse_ctx_t *pctx) {
   resyncbf(
      pctx,
      ResyncAct | ResyncScene | ResyncEnter
      | ResyncExit | ResyncExeunt | ResyncLine
   );
}

static void resyncfs(parse_ctx_t *pctx, const char *follow_set[]) {
   pctx->reason = "resync failed";
   for (;;) {
      for (size_t i = 0; follow_set[i] != NULL; i++) {
         if (!strcasecmp(pctx->tok->run, follow_set[i])) {
            return;
         }
      }
      gettok(pctx);
   }
}

static inline void rewind_tokstate(parse_ctx_t *pctx, size_t orig_idx) {
   pctx->idx = orig_idx;
   pctx->tok = array_peek(pctx->toks, pctx->idx);
}

static void regerr(parse_ctx_t *pctx) {
   synerr_t err;

   err.etok = pctx->etok;
   err.reason = pctx->reason;

   array_append(pctx->errs, &err, sizeof err);

   pctx->errcnt++;

   if (pctx->errcnt == pctx->errcnt_max) {
      longjmp(pctx->env_parse, NODEKIND__FINALE);
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

   /* " hero and thyself?" <- line
             ^^^           <- token to be highlighted
       123456789012345678  <- lpos
       012345678901234567  <- idx */

   safe_vprintf(
      Cbred "<SYNTAX ERROR #%zu>" Creset " %s\n",
      idx + 1, err->reason
   );
   safe_vprintf(
      "[%s:%zu:%zu] " Cbwhite "note:" Creset " found here\n",
      pctx->sourcefile, lnum, lpos
   );
   safe_vprintf(  /* " hero " */
      "%4d|%.*s",
      lnum, lpos - 1, l->run
   );
   safe_vprintf(  /* "and" */
      Cbblue "%s" Creset,
      err->etok->run
   );
   safe_vprintf(  /* "thyself?" */
      "%s\n",
      &l->run[lpos - 1 + err->etok->len]
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
      pctx->sourcefile, lnum, lpos,
      lnum, lpos - 1, l->run, &l->run[lpos - 1]
   );
}

[[maybe_unused]]
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
      pctx->sourcefile, lnum, lpos,
      lnum, lpos - 1, l->run, &l->run[lpos - 1]
   );
   exit(EXIT_FAILURE);
}

static tree_t *plant_tree(
   parse_ctx_t *pctx,
   const char *run,
   size_t len,
   nodekind_t kind,
   size_t lnum,
   size_t lpos
) {
   node_t node;
   char *buf;

   if (run) {
      // TODO: it'd be nice to have arena_strdup
      buf = arena_malloc_fallback(pctx->arena, len + 1);  /* +1 for \0 */
      memcpy(buf, run, len + 1);  /* \0 copied */
   }
   else buf = NULL;

   node.datkind = DATKIND_STR;
   node.dat.s.run = buf;
   node.dat.s.len = len;
   node.kind = kind;
   node.lnum = lnum;
   node.lpos = lpos;

   return tree_plant(&node, sizeof node, pctx->arena);
}

static tree_t *graft_tree_s(
   parse_ctx_t *pctx,
   tree_t *base,
   nodekind_t kind,
   const token_t *tok
) {
   tree_t *sub = plant_tree(
      pctx,
      tok->run,
      tok->len,
      kind,
      tok->lnum,
      tok->lpos
   );

   return tree_graft(base, sub);
}

static tree_t *graft_tree_n(
   parse_ctx_t *pctx,
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
   sub = tree_plant(&node, sizeof node, pctx->arena);

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

[[maybe_unused]]
extern void showpt(compile_ctx_t *cctx) {
   tree_pre_traverse(cctx->pt, shownode, 0, NULL);
}

static bool shownode(tree_t *t, size_t lv, void *ctx) {
   static char buf[128];

   node_t *n;
   int cnt, total;

   (void) ctx;
   n = tree_dat(t);
   cnt = sprintf(buf, "%zu", lv);
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

   return true;
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
