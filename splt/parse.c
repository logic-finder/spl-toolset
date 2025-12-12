#include "parse.h"
#include "parse.type.h"

/******************************
 * IMPORTANT GLOBAL VARIABLES *
 ******************************/
/* Token Stream */
static arr_t *toks;
static token_t
   *tok,    // toks[idx]
   *etok;   // used in `tell()` for printing an error
static int
   len,     // toks.length
   idx,     // current index in toks
   tidx;    // temp. var. for idx

/* Parse tree */
static tree_t
   *pt,     // parse tree
   *act,    // current act
   *scene,  // current scene
   *line;   // current line

/* Miscellaneous */
static jmp_buf LONGJMP_ENV;        // for setjmp & longjmp

/*
 * `seek_if` and `parse_if` being the
 * first element is intentional;
 * refer to `parse_line_as_conseq`.
 */
static const stmthandler_t stmts[] = {
   { seek_if  , parse_if   },
   { seek_asgn, parse_asgn },
   { seek_out , parse_out  },
   { seek_in  , parse_in   },
   { seek_goto, parse_goto },
   { seek_cond, parse_cond },
   { seek_push, parse_push },
   { seek_pop , parse_pop  }
};
static const int stmts_len = ARRLEN(stmts);

extern tree_t *parse(
   optflg_t *of,
   optval_t *ov,
   arr_t *tokens
) {
   // Initialize global variables
   toks = tokens;
   tok = arr_peek(toks, 0);
   len = arr_size(toks);
   idx = -1;
   pt = plant_tree(NULL, 0, NODEKIND_ROOT, 0, 0);

   // Construct the parse tree
   act = scene = line = NULL;

   parse_title();
   parse_dp();
   for (;;) {
      parse_act();
      for (;;) {
         parse_scene();
         switch (parse_stmt()) {
            case NODEKIND_SCENE   : goto EOS;
            case NODEKIND_ACT     : goto EOA;
            case NODEKIND__FINALE : goto EOE;
         }
         EOS:;
      }
      EOA:;
   }
   EOE:;

   // Cleanup
   arr_foreach(toks, &cleanup_tokstream);
   arr_destroy(toks);

   return pt;
}

static void cleanup_tokstream(void *tok, int _) {
   (void) _;
   free(((token_t *) tok)->run);
}

static void parse_title(void) {
   tree_t *title;

   reason = msgs.err.syn.title.incomp;
   title = graft_tree(pt, NULL, 0, NODEKIND_TITLE);
   readtoks('.', title);
}

static void parse_dp(void) {
   tree_t *dp, *character;

   reason = msgs.err.syn.dp.incomp;
   dp = graft_tree(pt, NULL, 0, NODEKIND_DP);
   for (;;) {
      reason = msgs.err.syn.dp.incomp;
      gettok();
      reason = msgs.err.syn.dp.noname;
      eqtok(',');
      character = graft_tree(dp, NULL, 0, NODEKIND_CHAR);
      ungettok();
      reason = msgs.err.syn.dp.chardecl_incomp;
      readtoks(',', character);
      reason = msgs.err.syn.dp.desc_incomp;
      skiptoks('.');
      reason = msgs.err.syn.dp.nonext;
      gettok();
      if (seek_act()) break;
      ungettok();
   }
}

static int seek_act(void) {
   return strcmp(tok->run, KEYWRD_ACT) ? 0 : 1;
}

static void parse_act(void) {
   reason = msgs.err.syn.act.incomp;
   gettok();
   act = graft_tree(pt, tok->run, tok->len, NODEKIND_ACT);
   gettok();
   reason = msgs.err.syn.act.badsyn;
   neqtok(':');
   reason = msgs.err.syn.act.desc_incomp;
   skiptoks('.');
   reason = msgs.err.syn.act.noscene;
   gettok();
   if (!seek_scene()) synerr();
}

static int seek_scene(void) {
   return strcmp(tok->run, KEYWRD_SCENE) ? 0 : 1;
}

static void parse_scene(void) {
   reason = msgs.err.syn.scene.incomp;
   gettok();
   scene = graft_tree(act, tok->run, tok->len, NODEKIND_SCENE);
   gettok();
   reason = msgs.err.syn.scene.badsyn;
   neqtok(':');
   reason = msgs.err.syn.scene.desc_incomp;
   skiptoks('.');
}

static int seek_enterlike(const char *type) {
   if (tok->run[0] != '[')
      return 0;
   gettok();
   if (strcmp(tok->run, type))
      return 0;
   else
      return 1;
}

static int seek_enter(void) {
   return seek_enterlike(KEYWRD_ENTER);
}

static void parse_enter(void) {
   tree_t *enter, *character;

   reason = msgs.err.syn.enter.incomp;
   enter = graft_tree(scene, NULL, 0, NODEKIND_ENTER);
   character = graft_tree(enter, NULL, 0, NODEKIND_CHAR);

   // Enter has 1 or 2 characters
   for (;;) {
      gettok();
      if (tok->run[0] == ']')
         break;
      if (!strcmp(tok->run, KEYWRD_AND)) {
         character = graft_tree(enter, NULL, 0, NODEKIND_CHAR);
         continue;
      }
      (void) graft_tree(
         character, tok->run, tok->len, NODEKIND_DATA);
   }
   if (!tree_clen(enter)) {
      reason = msgs.err.syn.enter.nochar;
      synerr();
   }
}

static int seek_exit(void) {
   return seek_enterlike(KEYWRD_EXIT);
}

static void parse_exit(void) {
   tree_t *exit, *character;

   reason = msgs.err.syn.exit.incomp;
   exit = graft_tree(scene, NULL, 0, NODEKIND_EXIT);
   character = graft_tree(exit, NULL, 0, NODEKIND_CHAR);

   // Exit has 1 character
   readtoks(']', character);

   if (!tree_clen(exit)) {
      reason = msgs.err.syn.exit.nochar;
      synerr();
   }
}

static int seek_exeunt(void) {
   return seek_enterlike(KEYWRD_EXEUNT);
}

static void parse_exeunt(void) {
   tree_t *exeunt, *character;
   int planted;

   reason = msgs.err.syn.exeunt.incomp;
   exeunt = graft_tree(scene, NULL, 0, NODEKIND_EXEUNT);

   // Exeunt has either 0 or 2 characters
   planted = 0;
   for (;;) {
      gettok();
      if (tok->run[0] == ']')
         break;
      if (!strcmp(tok->run, KEYWRD_AND)) {
         planted = 1 - planted;
         continue;
      }
      if (!planted) {
         character = graft_tree(exeunt, NULL, 0, NODEKIND_CHAR);
         planted = 1 - planted;
      }
      (void) graft_tree(
         character, tok->run, tok->len, NODEKIND_DATA);
   }
   if (tree_clen(exeunt) == 1) {
      reason = msgs.err.syn.exeunt.onechar;
      synerr();
   }
}

static void parse_const(tree_t *stmt) {
   static const char *decos[] = {
      /* possessives */
      KEYWRD_MY, KEYWRD_YOUR, KEYWRD_THY, KEYWRD_THINE,
      KEYWRD_HIS, KEYWRD_HER, KEYWRD_ITS,
      /* articles */
      KEYWRD_A, KEYWRD_AN, KEYWRD_THE
   };
   static const int decos_len = ARRLEN(decos);

   bool opcond, cond1, cond2, cond3, cond4;
   int ret;
   nodekind_t kind;

   reason = msgs.err.syn.cnst.incomp;
   gettok();
   ret = match_str(tok->run, decos, decos_len);
   if (ret >= 0)
      gettok();

   opcond = (kind = seek_op()) != NODEKIND__NAO;
   if (opcond) {
      parse_op(stmt, kind);
      return;
   }

   for (;;) {
      // escape conditions
      cond1 = match(tok->run[0], ".!?");
      cond2 = !strcmp(tok->run, KEYWRD_AND);
      cond3 = !strcmp(tok->run, KEYWRD_NOT);
      cond4 = !strcmp(tok->run, KEYWRD_THAN);
      if (cond1 || cond2 || cond3)
         break;
      if (cond4) {
         ungettokn(2);
         break;
      }

      // is it a decorator?
      if (match_str(tok->run, decos, decos_len) >= 0) {
         reason = msgs.err.syn.cnst.deco;
         synerr();
      }

      (void) graft_tree(stmt, tok->run, tok->len, NODEKIND_ADJ);

      reason = msgs.err.syn.cnst.incomp;
      gettok();
   }
   TREE_CHDAT(stmt, tree_clen(stmt) - 1)->kind = NODEKIND_NOUN;
}

static nodekind_t seek_op(void) {
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
      { KEYWRD_2X   , NODEKIND_2X   },
      { KEYWRD_FACT , NODEKIND_FACT }
   };
   static int ops_len = ARRLEN(ops);

   const ophandler_t *op;
   int i;
   nodekind_t k;

   k = NODEKIND__NAO;
   for (i = 0; i < ops_len; i++) {
      op = ops + i;
      if (!strcmp(tok->run, op->name)) {
         k = op->kind; break;
      }
   }

   if (k == NODEKIND__NAO || k != NODEKIND_SQUR)
      return k;

   gettok();
   if (!strcmp(tok->run, KEYWRD_ROOT)) {
      k = NODEKIND_SQRT;
   }
   else ungettok();

   return k;
}

static void parse_op(tree_t *stmt, nodekind_t kind) {
   tree_t *op;

   reason = msgs.err.syn.op.incomp;
   op = graft_tree(stmt, NULL, 0, kind);

   switch (kind) {
      case NODEKIND_SUM  : parse_op_sum (op); return;
      case NODEKIND_DIFF : parse_op_diff(op); return;
      case NODEKIND_PROD : parse_op_prod(op); return;
      case NODEKIND_QUOT : parse_op_quot(op); return;
      case NODEKIND_REM  : parse_op_rem (op); return;
      case NODEKIND_SQRT : parse_op_sqrt(op); return;
      case NODEKIND_SQUR : parse_op_squr(op); return;
      case NODEKIND_CUBE : parse_op_cube(op); return;
      case NODEKIND_2X   : parse_op_2x  (op); return;
      case NODEKIND_FACT : parse_op_fact(op); return;
      /* control never reaches here */
      default : ;
   }
}

static void parse_op_unary(
   tree_t *op,
   const char *type,
   const char *err
) {
   reason = msgs.err.syn.op.incomp;
   gettok();
   if (strcmp(tok->run, type)) {
      reason = err;
      synerr();
   }
   parse_const(op);
}

static void parse_op_binary(
   tree_t *op,
   const char *type,
   const char *err
) {
   tree_t *lefthand, *righthand;

   reason = msgs.err.syn.op.incomp;
   gettok();
   if (strcmp(tok->run, type)) {
      reason = err;
      synerr();
   }

   lefthand = graft_tree(op, NULL, 0, NODEKIND_LHS);
   parse_const(lefthand);
   righthand = graft_tree(op, NULL, 0, NODEKIND_RHS);
   parse_const(righthand);
}

static void parse_op_sum(tree_t *op) {
   /*
    * the sum of <const> and <const>
    */
   parse_op_binary(op, KEYWRD_OF, msgs.err.syn.op.sum);
}

static void parse_op_diff(tree_t *op) {
   /*
    * the difference between <const> and <const>
    */
   parse_op_binary(op, KEYWRD_BTW, msgs.err.syn.op.diff);
}

static void parse_op_prod(tree_t *op) {
   /*
    * the product of <const> and <const>
    */
   parse_op_binary(op, KEYWRD_OF, msgs.err.syn.op.prod);
}

static void parse_op_quot(tree_t *op) {
   /*
    * the quotient between <const> and <const>
    */
   parse_op_binary(op, KEYWRD_BTW, msgs.err.syn.op.quot);
}

static void parse_op_rem(tree_t *op) {
   /*
    * the remainder of
    *    the quotient between <const> and <const>
    */
   reason = msgs.err.syn.op.incomp;
   gettok();
   if (strcmp(tok->run, KEYWRD_OF)) {
      reason = msgs.err.syn.op.rem;
      synerr();
   }

   gettok();
   if (strcmp(tok->run, KEYWRD_THE)) {
      reason = msgs.err.syn.op.rem_quot_1;
      synerr();
   }

   gettok();
   if (strcmp(tok->run, KEYWRD_QUOT)) {
      reason = msgs.err.syn.op.rem_quot_2;
      synerr();
   }

   parse_op_quot(op);
}

static void parse_op_sqrt(tree_t *op) {
   /*
    * the square root of <const>
    */
   parse_op_unary(op, KEYWRD_OF, msgs.err.syn.op.sqrt);
}

static void parse_op_squr(tree_t *op) {
   /*
    * the square of <const>
    */
   parse_op_unary(op, KEYWRD_OF, msgs.err.syn.op.squr);
}

static void parse_op_cube(tree_t *op) {
   /*
    * the cube of <const>
    */
   parse_op_unary(op, KEYWRD_OF, msgs.err.syn.op.cube);
}

static void parse_op_2x(tree_t *op) {
   parse_const(op);
}

static void parse_op_fact(tree_t *op) {
   /*
    * the factorial of <const>
    */
   parse_op_unary(op, KEYWRD_OF, msgs.err.syn.op.fact);
}

static int seek_line(void) {
   tree_t *dp, *character;
   int i, k, dp_clen, char_clen;

   dp = tree_child(pt, 1);
   dp_clen = tree_clen(dp);

   for (i = 0; i < dp_clen; i++) {
      character = tree_child(dp, i);
      char_clen = tree_clen(character);
      for (k = 0; k < char_clen; k++) {
         if (strcmp(TREE_CHDAT(character, k)->run, tok->run))
            break;
         gettok();
      }
      rewind_tokstate();
      if (k == char_clen)
         return 1;
   }
   return 0;
}

static void parse_line(void) {
   tree_t *character;

   reason = msgs.err.syn.line.incomp;
   line = graft_tree(scene, NULL, 0, NODEKIND_LINE);
   character = graft_tree(line, NULL, 0, NODEKIND_CHAR);

   /*
    * We've already checked the character name in
    * `seek_line`, so no need to error-check here.
    */
   ungettok();
   readtoks(':', character);

   // Handle the first statement
   gettok();
   archive_tokstate();
   if (parse_line_router(stmts, stmts_len)) {
      reason = msgs.err.syn.line.nostmt;
      synerr();
   }

   // Handle the rest
   for (;;) {
      if (idx == len - 1)
         JUMP(NODEKIND__FINALE);
      /*
       * if this token is the beginning of another line,
       * then longjmp happens inside `seek_stmt_router`.
       */
      reason = msgs.err.syn.eot;
      gettok();
      archive_tokstate();
      seek_stmt_router();
      if (parse_line_router(stmts, stmts_len)) {
         reason = msgs.err.syn.line.nostmt;
         synerr();
      }
   }
}

static int parse_line_as_conseq(void) {
   /*
    * Since it is a consequent, I think we should
    * rule out 'seek_if' and 'parse_if'.
    */
   gettok();
   archive_tokstate();

   if (isupper(tok->run[0]))
      return 2;
   else
   if (islower(tok->run[0]))
      tok->run[0] = toupper(tok->run[0]);

   return parse_line_router(stmts + 1, stmts_len - 1);
}

static int parse_line_router(
   const stmthandler_t stmts[static 6],
   int stmts_len
) {
   const stmthandler_t *stmt;
   int i;

   for (i = 0; i < stmts_len; i++) {
      stmt = stmts + i;
      if ((*stmt->seek)()) {
         (*stmt->parse)();
         break;
      }
      rewind_tokstate();
   }

   return i == stmts_len ? 1 : 0;
}

static int seek_asgn(void) {
   if (strcmp(tok->run, KEYWRD_YOU) && strcmp(tok->run, KEYWRD_THOU))
      return 0;
   return 1;
}

static void parse_asgn(void) {
   int type;
   bool cond1, cond2;

   if (!strcmp(tok->run, KEYWRD_YOU))
      type = 1;
   else  /* Thou */
      type = 2;

   reason = msgs.err.syn.asgn.incomp;
   gettok();

   cond1 = !strcmp(tok->run, KEYWRD_ARE);
   cond2 = !strcmp(tok->run, KEYWRD_ART);

   if ((type == 1 && cond2) || (type == 2 && cond1)) {
      reason = msgs.err.syn.asgn.not_conj;
      synerr();
   }

   if (cond1 || cond2)
      parse_asgn_i();   // You are as ...
   else {
      ungettok();
      parse_asgn_ii();  // You ... sth!
   }
}

static void parse_asgn_i(void) {
   tree_t *asgn_i;

   reason = msgs.err.syn.asgn.incomp;
   gettok();
   if (strcmp(tok->run, KEYWRD_AS)) {
      reason = msgs.err.syn.asgn.no_as;
      synerr();
   }

   gettok();
   if (tok->kind == TOKKIND_PNT) {
      reason = msgs.err.syn.asgn.no_adj;
      synerr();
   }

   asgn_i = graft_tree(line, tok->run, tok->len, NODEKIND_ASSIGN);

   gettok();
   if (strcmp(tok->run, KEYWRD_AS)) {
      reason = msgs.err.syn.asgn.no_as;
      synerr();
   }

   parse_const(asgn_i);
}

static void parse_asgn_ii(void) {
   tree_t *asgn_ii;

   asgn_ii = graft_tree(line, NULL, 0, NODEKIND_ASSIGN);
   parse_const(asgn_ii);
}

static void seek_stmt(void) {
   if (idx == len - 1)
      JUMP(NODEKIND__FINALE);

   reason = msgs.err.syn.eot;
   gettok();
   archive_tokstate();
   seek_stmt_router();
   reason = msgs.err.syn.incomprehensible;
   synerr();
}

static void seek_stmt_router(void) {
   typedef struct jumper {
      seeker_t *seek;
      int retval;
   } jumper_t;

   static const jumper_t jps[] = {
      { seek_enter , NODEKIND_ENTER  },
      { seek_exit  , NODEKIND_EXIT   },
      { seek_exeunt, NODEKIND_EXEUNT },
      { seek_scene , NODEKIND_SCENE  },
      { seek_act   , NODEKIND_ACT    },
      { seek_line  , NODEKIND_LINE   },
   };
   static const int jps_len = ARRLEN(jps);

   const jumper_t *jp;

   for (int i = 0; i < jps_len; i++) {
      jp = jps + i;
      if ((*jp->seek)())
         JUMP(jp->retval);
      rewind_tokstate();
   }
}

static int parse_stmt(void) {
   int ret;

   ret = setjmp(LONGJMP_ENV);
   switch (ret) {
      case NODEKIND_ENTER  : parse_enter();  break;
      case NODEKIND_EXIT   : parse_exit();   break;
      case NODEKIND_EXEUNT : parse_exeunt(); break;
      case NODEKIND_LINE   : parse_line();   break; /* seek_stmt */
      case NODEKIND_SCENE   : /* fall-through */
      case NODEKIND_ACT     : /* fall-through */
      case NODEKIND__FINALE : return ret;
      case NODEKIND__SETJMP : ; /* first setjmp call */
   }
   seek_stmt(); /* longjmp */

   return 0;  /* control never reaches here */
}

static int seek_out(void) {
   /*
    * Need to check the third token because of
    * the input statement "Open your mind".
    */
   if (!strcmp(tok->run, KEYWRD_OPEN)) {
      gettokn(2);
      if (strcmp(tok->run, KEYWRD_HEART))
         return 0;
      ungettokn(2);
      return 1;
   }
   else
   if (!strcmp(tok->run, KEYWRD_SPEAK))
      return 1;
   else
      return 0;
}

static void parse_out(void) {
   token_t *prev;
   nodekind_t kind;
   bool mcond, lcond1, lcond2;
   int type;

   if (!strcmp(tok->run, KEYWRD_OPEN))
      type = 1, kind = NODEKIND_OUT_N;
   else  /* Speak */
      type = 2, kind = NODEKIND_OUT_C;

   reason = msgs.err.syn.out.incomp;
   (void) graft_tree(line, NULL, 0, kind);

   gettok();
   prev = tok;
   gettok();

   mcond = !strcmp(prev->run, KEYWRD_YOUR)
      || !strcmp(prev->run, KEYWRD_YOUR_U)
      || !strcmp(prev->run, KEYWRD_THY);
   lcond1 = !strcmp(tok->run, KEYWRD_HEART);
   lcond2 = !strcmp(tok->run, KEYWRD_MIND);

   if (!mcond) {
      reason = msgs.err.syn.out.badsyn;
      etok = prev;
      synerr();
   }
   if ((type == 1 && lcond2) || (type == 2 && lcond1)) {
      reason = msgs.err.syn.out.unmatched;
      synerr();
   }

   gettok();
   if (!match(tok->run[0], ".!")) {
      reason = msgs.err.syn.out.badsyn;
      synerr();
   }
}

static int seek_in(void) {
   /*
    * Since `seek_out` is executed first,
    * no need to check the third token.
    * Refer to `seek_stmt`.
    */
   if (strcmp(tok->run, KEYWRD_LISTEN) && strcmp(tok->run, KEYWRD_OPEN))
      return 0;
   else
      return 1;
}

static void parse_in(void) {
   token_t *prev;
   nodekind_t kind;
   bool mcond, lcond1, lcond2;
   int type;

   if (!strcmp(tok->run, KEYWRD_LISTEN))
      type = 1, kind = NODEKIND_IN_N;
   else  /* Open */
      type = 2, kind = NODEKIND_IN_C;

   reason = msgs.err.syn.in.incomp;
   (void) graft_tree(line, NULL, 0, kind);

   if (type == 1) {
      gettok();
      if (strcmp(tok->run, KEYWRD_TO)) {
         reason = msgs.err.syn.in.badsyn;
         synerr();
      }
   }

   gettok();
   prev = tok;
   gettok();

   mcond = !strcmp(prev->run, KEYWRD_YOUR)
           || !strcmp(prev->run, KEYWRD_YOUR_U);
   lcond1 = !strcmp(tok->run, KEYWRD_HEART);
   lcond2 = !strcmp(tok->run, KEYWRD_MIND);

   if (!mcond) {
      reason = msgs.err.syn.in.badsyn;
      synerr();
   }
   if ((type == 1 && lcond2) || (type == 2 && lcond1)) {
      reason = msgs.err.syn.in.unmatched;
      synerr();
   }

   gettok();
   if (!match(tok->run[0], ".!")) {
      reason = msgs.err.syn.in.badsyn;
      synerr();
   }
}

static int seek_goto(void) {
   if (strcmp(tok->run, KEYWRD_LET) && strcmp(tok->run, KEYWRD_WE))
      return 0;
   return 1;
}

static void parse_goto(void) {
   tree_t *gt;
   int type;
   bool cond1, cond2;

   if (!strcmp(tok->run, KEYWRD_LET))
      type = 1;
   else  /* we */
      type = 2;

   reason = msgs.err.syn.gt.incomp;
   gettok();
   cond1 = !strcmp(tok->run, KEYWRD_US);
   cond2 = !strcmp(tok->run, KEYWRD_SHALL)
           || !strcmp(tok->run, KEYWRD_MUST);

   if (!cond1 && !cond2) {
      reason = msgs.err.syn.gt.badsyn;
      synerr();
   }
   if ((type == 1 && cond2) || (type == 2 && cond1)) {
      reason = msgs.err.syn.gt.unmatched;
      synerr();
   }

   gettok();
   cond1 = strcmp(tok->run, KEYWRD_RETURN);
   cond2 = strcmp(tok->run, KEYWRD_PROCED);

   if (cond1 && cond2) {
      reason = msgs.err.syn.gt.badsyn;
      synerr();
   }

   gettok();
   if (strcmp(tok->run, KEYWRD_TO)) {
      reason = msgs.err.syn.gt.badsyn;
      synerr();
   }

   gettok();
   if (strcmp(tok->run, KEYWRD_SCENE)) {
      if (!strcmp(tok->run, "scene"))
         reason = msgs.err.syn.gt.misspell;
      else
         reason = msgs.err.syn.gt.badsyn;
      synerr();
   }

   gettok();
   if (tok->kind == TOKKIND_PNT) {
      reason = msgs.err.syn.gt.badsyn;
      synerr();
   }

   (void) graft_tree(line, tok->run, tok->len, NODEKIND_GOTO);

   gettok();
   if (!match(tok->run[0], ".!")) {
      reason = msgs.err.syn.in.badsyn;
      synerr();
   }
}

static int seek_cond(void) {
   bool cond1, cond2, cond3, cond4;

   cond1 = strcmp(tok->run, KEYWRD_AM);
   cond2 = strcmp(tok->run, KEYWRD_ARE_C);
   cond3 = strcmp(tok->run, KEYWRD_ART_C);
   cond4 = strcmp(tok->run, KEYWRD_IS);

   if (cond1 && cond2 && cond3 && cond4)
      return 0;
   else
      return 1;
}

static void parse_cond(void) {
   tree_t *condition, *lefthand, *righthand;
   const char *person;
   bool cond1, cond2, cond3;
   int type;

   reason = msgs.err.syn.cond.incomp;
   condition = graft_tree(line, NULL, 0, NODEKIND_COND);

   if (!strcmp(tok->run, KEYWRD_AM))
      type = 1, person = "1st";
   else
   if (!strcmp(tok->run, KEYWRD_ARE_C))
      type = 2, person = "2nd";
   else
   if (!strcmp(tok->run, KEYWRD_ART_C))
      type = 3, person = "2nd";
   else
   if (!strcmp(tok->run, KEYWRD_IS))
      type = 4, person = "3rd";

   if (type < 4) {
      gettok();
      cond1 = !strcmp(tok->run, KEYWRD_I);
      cond2 = !strcmp(tok->run, KEYWRD_YOU_L);
      cond3 = !strcmp(tok->run, KEYWRD_THOU_L);

      cond1 = type == 1 && !cond1;
      cond2 = type == 2 && !cond2;
      cond3 = type == 3 && !cond3;

      if (cond1 || cond2 || cond3) {
         reason = msgs.err.syn.cond.unmatched;
         synerr();
      }
   }
   else parse_const(condition);

   lefthand  = graft_tree(condition, NULL, 0, NODEKIND_LHS);
   (void) graft_tree(
      lefthand, person, strlen(person) + 1, NODEKIND_PERSON);

   gettok();
   if (!strcmp(tok->run, KEYWRD_NOT)) {
      (void) graft_tree(condition, NULL, 0, NODEKIND_NEGATE);
      gettok();
   }
   else
      (void) graft_tree(condition, NULL, 0, NODEKIND_AFFIRM);

   righthand = graft_tree(condition, NULL, 0, NODEKIND_RHS);

   if (!strcmp(tok->run, KEYWRD_AS)) {
      gettok();
      if (tok->kind == TOKKIND_PNT) {
         reason = msgs.err.syn.cond.badsyn;
         synerr();
      }

      (void) graft_tree(
         condition, tok->run, tok->len, NODEKIND_EQ);

      gettok();
      if (strcmp(tok->run, KEYWRD_AS)) {
         reason = msgs.err.syn.cond.badsyn;
         synerr();
      }
   }
   else {
      (void) graft_tree(
         condition, tok->run, tok->len, NODEKIND_INEQ);

      gettok();
      if (type < 4 && strcmp(tok->run, KEYWRD_THAN)) {
         reason = msgs.err.syn.cond.badsyn;
         synerr();
      }
   }

   parse_const(righthand);
}

static int seek_if(void) {
   return strcmp(tok->run, KEYWRD_IF) ? 0 : 1;
}

static void parse_if(void) {
   tree_t *ifstmt;
   int ret;

   reason = msgs.err.syn.ifstmt.incomp;
   ifstmt = graft_tree(line, NULL, 0, NODEKIND_IF);
   gettok();

   if (!strcmp(tok->run, KEYWRD_SO))
      (void) graft_tree(ifstmt, NULL, 0, NODEKIND_AFFIRM);
   else
   if (!strcmp(tok->run, KEYWRD_NOT))
      (void) graft_tree(ifstmt, NULL, 0, NODEKIND_NEGATE);
   else {
      reason = msgs.err.syn.ifstmt.badsyn;
      synerr();
   }

   gettok();
   if (tok->run[0] != ',') {
      reason = msgs.err.syn.ifstmt.badsyn;
      synerr();
   }

   // Parse the consequent
   reason = msgs.err.syn.ifstmt.conseq_incomp;
   ret = parse_line_as_conseq();

   if (!ret)
      return;

   switch (ret) {
      case 1: reason = msgs.err.syn.ifstmt.bad_conseq; break;
      case 2: reason = msgs.err.syn.ifstmt.conseq_cap; break;
   }
   synerr();
}

static int seek_push(void) {
   return strcmp(tok->run, KEYWRD_REMEMB) ? 0 : 1;
}

static void parse_push(void) {
   reason = msgs.err.syn.push.incomp;
   gettok();

   if (tok->kind == TOKKIND_PNT) {
      reason = msgs.err.syn.push.badsyn;
      synerr();
   }

   (void) graft_tree(line, tok->run, tok->len, NODEKIND_PUSH);

   gettok();
   if (!match(tok->run[0], ".!")) {
      reason = msgs.err.syn.in.badsyn;
      synerr();
   }
}

static int seek_pop(void) {
   return strcmp(tok->run, KEYWRD_RECALL) ? 0 : 1;
}

static void parse_pop(void) {
   reason = msgs.err.syn.pop.incomp;
   (void) graft_tree(line, NULL, 0, NODEKIND_POP);
   skiptoks2(".!?");
}

static void nexttok(void) {
   if (++idx == len) synerr();
   tok = arr_peek(toks, idx);
}

static void gettok(void) {
   nexttok();
   etok = tok;
}

static void gettokn(int n) {
   if (idx + n >= len) synerr();
   idx += n;
   tok = arr_peek(toks, idx);
   etok = tok;
}

static void ungettok(void) {
   tok = arr_peek(toks, --idx);
}

static void ungettokn(int n) {
   idx -= n;
   tok = arr_peek(toks, idx);
}

static void skiptoks(char sentinel) {
   for (;;) {
      nexttok();
      if (tok->run[0] == sentinel)
         break;
   }
   etok = tok;
}

static void skiptoks2(const char *sentinels) {
   for (;;) {
      nexttok();
      if (match(tok->run[0], sentinels))
         break;
   }
   etok = tok;
}

static void eqtok(char ch) {
   if (tok->run[0] != ch)
      return;
   etok = tok;
   synerr();
}

static void neqtok(char ch) {
   if (tok->run[0] == ch)
      return;
   etok = tok;
   synerr();
}

static void readtoks(char sentinel, tree_t *base) {
   for (;;) {
      nexttok();
      if (tok->run[0] == sentinel)
         return;
      (void) graft_tree(
         base, tok->run, tok->len, NODEKIND_DATA);
   }
   etok = tok;
}

static inline void archive_tokstate(void) {
   tidx = idx;
}

static inline void rewind_tokstate(void) {
   idx = tidx;
   tok = arr_peek(toks, idx);
}

static inline void synerr(void) {
   err_template(tell, Cbred, "<syntax error> ");
}

static void tell(void) {
   int lnum, lpos;
   line_t *l;

   lnum = etok->lnum;
   lpos = etok->lpos;
   l = arr_peek(ls, lnum - 1);

   ffmtwrt(stderr,
      "%s\n"
      "[%s:%d:%d] " Cbwhite "note:" Creset " problematic since here\n"
      "%4d|%.*s" Cbblue "%s" Creset "\n",
      reason,
      sfname, lnum, lpos,
      lnum, lpos - 1, l->run, &l->run[lpos - 1]
   );
}

static tree_t *plant_tree(
   const char *run,
   int len,
   nodekind_t kind,
   int lnum,
   int lpos
) {
   node_t node;
   char *buf;

   if (run) {
      buf = smalloc(len);  /* already has a room for 0 */
      strcpy(buf, run);
   }
   else buf = NULL;

   node.run = buf;
   node.len = len;
   node.kind = kind;
   node.lnum = lnum;
   node.lpos = lpos;

   return tree_plant(&node, sizeof node);
}

static tree_t *graft_tree(
   tree_t *base,
   const char *run,
   int len,
   nodekind_t kind
) {
   tree_t *ret, *sub;

   sub = plant_tree(
      run,
      len,
      kind,
      tok->lnum,
      tok->lpos
   );
   ret = tree_graft(base, sub);

   return ret;
}
