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
static const stmtkind_t stmtkind;  // kind of stmt

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
   tok = NULL;
   len = arr_size(toks);
   idx = -1;
   pt = plant_tree(NULL, 0, "ROOT", 0, 0);

   // Construct the parse tree
   act = scene = line = NULL;

   parse_title();
   parse_dp();
   for (;;) {
      parse_act();
      for (;;) {
         parse_scene();
         switch (parse_stmt()) {
            case STMTKIND_SCENE  : goto EOS;
            case STMTKIND_ACT    : goto EOA;
            case STMTKIND_FINALE : goto EOE;
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
   title = graft_tree(pt, NULL, 0, "TITLE");
   readtoks('.', title);
}

static void parse_dp(void) {
   tree_t *dp, *character;

   reason = msgs.err.syn.dp.incomp;
   dp = graft_tree(pt, NULL, 0, "D.P.");
   for (;;) {
      reason = msgs.err.syn.dp.incomp;
      gettok();
      reason = msgs.err.syn.dp.noname;
      eqtok(',');
      character = graft_tree(dp, NULL, 0, "CHAR");
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
   return strcmp(tok->run, "Act") ? 0 : 1;
}

static void parse_act(void) {
   reason = msgs.err.syn.act.incomp;
   gettok();
   act = graft_tree(pt, tok->run, tok->len, "ACT");
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
   return strcmp(tok->run, "Scene") ? 0 : 1;
}

static void parse_scene(void) {
   reason = msgs.err.syn.scene.incomp;
   gettok();
   scene = graft_tree(act, tok->run, tok->len, "SCENE");
   gettok();
   reason = msgs.err.syn.scene.badsyn;
   neqtok(':');
   reason = msgs.err.syn.scene.desc_incomp;
   skiptoks('.');
}

static int seek_enter(void) {
   if (tok->run[0] != '[')
      return 0;
   gettok();
   if (strcmp(tok->run, "Enter"))
      return 0;
   else
      return 1;
}

static void parse_enter(void) {
   tree_t *enter, *character;

   reason = msgs.err.syn.enter.incomp;
   enter = graft_tree(scene, NULL, 0, "ENTER");
   character = graft_tree(enter, NULL, 0, "CHAR");

   // Enter has 1 or 2 characters
   for (;;) {
      gettok();
      if (tok->run[0] == ']')
         break;
      if (!strcmp(tok->run, "and")) {
         character = graft_tree(enter, NULL, 0, "CHAR");
         continue;
      }
      (void) graft_tree(
         character, tok->run, tok->len, "CONTENT");
   }
   if (!tree_clen(enter)) {
      reason = msgs.err.syn.enter.nochar;
      synerr();
   }
}

static int seek_exit(void) {
   if (tok->run[0] != '[')
      return 0;
   gettok();
   if (strcmp(tok->run, "Exit"))
      return 0;
   else
      return 1;
}

static void parse_exit(void) {
   tree_t *exit;

   reason = msgs.err.syn.exit.incomp;
   exit = graft_tree(scene, NULL, 0, "EXIT");

   // Exit has 1 character
   readtoks(']', exit);

   if (!tree_clen(exit)) {
      reason = msgs.err.syn.exit.nochar;
      synerr();
   }
}

static int seek_exeunt(void) {
   if (tok->run[0] != '[')
      return 0;
   gettok();
   if (strcmp(tok->run, "Exeunt"))
      return 0;
   else
      return 1;
}

static void parse_exeunt(void) {
   tree_t *exeunt, *character;
   int planted;

   reason = msgs.err.syn.exeunt.incomp;

   exeunt = graft_tree(scene, NULL, 0, "EXEUNT");

   // Exeunt has either 0 or 2 characters
   planted = 0;
   for (;;) {
      gettok();
      if (tok->run[0] == ']')
         break;
      if (!strcmp(tok->run, "and")) {
         planted = 1 - planted;
         continue;
      }
      if (!planted) {
         character = graft_tree(exeunt, NULL, 0, "CHAR");
         planted = 1 - planted;
      }
      (void) graft_tree(
         character, tok->run, tok->len, "CONTENT");
   }
   if (tree_clen(exeunt) == 1) {
      reason = msgs.err.syn.exeunt.onechar;
      synerr();
   }
}

static void parse_const(tree_t *stmt) {
   static const char *decos[] = {
      /* possessives */
      "my", "your", "thy", "thine",
      "his", "her", "its",
      /* articles */
      "a", "an", "the"
   };
   static const int decos_len = ARRLEN(decos);

   bool opcond, cond1, cond2, cond3, cond4;
   int ret;
   opkind_t kind;

   reason = msgs.err.syn.cnst.incomp;
   gettok();
   ret = match_tokrun(decos, decos_len);
   if (ret >= 0)
      gettok();

   opcond = (kind = seek_op()) != OPKIND_NAO;
   if (opcond) {
      parse_op(stmt, kind);
      return;
   }

   for (;;) {
      // escape conditions
      cond1 = match(tok->run[0], ".!?");
      cond2 = !strcmp(tok->run, "and");
      cond3 = !strcmp(tok->run, "not");
      cond4 = !strcmp(tok->run, "than");
      if (cond1 || cond2 || cond3)
         break;
      if (cond4) {
         ungettokn(2);
         break;
      }

      // is it a decorator?
      if (match_tokrun(decos, decos_len) >= 0) {
         reason = msgs.err.syn.cnst.deco;
         synerr();
      }

      (void) graft_tree(stmt, tok->run, tok->len, "ADJ");

      reason = msgs.err.syn.cnst.incomp;
      gettok();
   }
   strcpy(TREE_CHDAT(stmt, tree_clen(stmt) - 1)->tag, "NOUN");
}

static opkind_t seek_op(void) {
   typedef struct ophandler {
      const char *name;
      opkind_t kind;
   } ophandler_t;

   static ophandler_t ops[] = {
      { "sum"       , OPKIND_SUM  },
      { "difference", OPKIND_DIFF },
      { "product"   , OPKIND_PROD },
      { "quotient"  , OPKIND_QUOT },
      { "remainder" , OPKIND_REM  },
      { "square"    , OPKIND_SQUR },
      { "cube"      , OPKIND_CUBE },
      { "twice"     , OPKIND_2X   },
      { "factorial" , OPKIND_FACT }
   };
   static int ops_len = ARRLEN(ops);

   const ophandler_t *op;
   int i;
   opkind_t k;

   k = OPKIND_NAO;
   for (i = 0; i < ops_len; i++) {
      op = ops + i;
      if (!strcmp(tok->run, op->name)) {
         k = op->kind; break;
      }
   }

   if (k == OPKIND_NAO || k != OPKIND_SQUR)
      return k;

   gettok();
   if (!strcmp(tok->run, "root")) {
      k = OPKIND_SQRT;
   }
   else ungettok();

   return k;
}

static const char *get_opname(opkind_t kind) {
   switch (kind) {
      case OPKIND_SUM  : return "SUM" ;
      case OPKIND_DIFF : return "DIFF";
      case OPKIND_PROD : return "PROD";
      case OPKIND_QUOT : return "QUOT";
      case OPKIND_REM  : return "REM" ;
      case OPKIND_SQRT : return "SQRT";
      case OPKIND_SQUR : return "SQUR";
      case OPKIND_CUBE : return "CUBE";
      case OPKIND_2X   : return "2X"  ;
      case OPKIND_FACT : return "FACT";
      /* control never reaches here */
      case OPKIND_NAO  : goto
                           unreachable;
   }
   unreachable: return NULL;
}

static void parse_op(tree_t *stmt, opkind_t kind) {
   tree_t *op;
   const char *opname;

   reason = msgs.err.syn.op.incomp;
   opname = get_opname(kind);
   op = graft_tree(stmt, NULL, 0, opname);

   switch (kind) {
      case OPKIND_SUM  : parse_op_sum (op); return;
      case OPKIND_DIFF : parse_op_diff(op); return;
      case OPKIND_PROD : parse_op_prod(op); return;
      case OPKIND_QUOT : parse_op_quot(op); return;
      case OPKIND_REM  : parse_op_rem (op); return;
      case OPKIND_SQRT : parse_op_sqrt(op); return;
      case OPKIND_SQUR : parse_op_squr(op); return;
      case OPKIND_CUBE : parse_op_cube(op); return;
      case OPKIND_2X   : parse_op_2x  (op); return;
      case OPKIND_FACT : parse_op_fact(op); return;
      /* control never reaches here */
      case OPKIND_NAO  : ;
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

   lefthand = graft_tree(op, NULL, 0, "LHS");
   parse_const(lefthand);
   righthand = graft_tree(op, NULL, 0, "RHS");
   parse_const(righthand);
}

static void parse_op_sum(tree_t *op) {
   /*
    * the sum of <const> and <const>
    */
   parse_op_binary(op, "of", msgs.err.syn.op.sum);
}

static void parse_op_diff(tree_t *op) {
   /*
    * the difference between <const> and <const>
    */
   parse_op_binary(op, "between", msgs.err.syn.op.diff);
}

static void parse_op_prod(tree_t *op) {
   /*
    * the product of <const> and <const>
    */
   parse_op_binary(op, "of", msgs.err.syn.op.prod);
}

static void parse_op_quot(tree_t *op) {
   /*
    * the quotient between <const> and <const>
    */
   parse_op_binary(op, "between", msgs.err.syn.op.quot);
}

static void parse_op_rem(tree_t *op) {
   /*
    * the remainder of
    *    the quotient between <const> and <const>
    */
   tree_t *rem_quot;

   reason = msgs.err.syn.op.incomp;
   gettok();
   if (strcmp(tok->run, "of")) {
      reason = msgs.err.syn.op.rem;
      synerr();
   }

   gettok();
   if (strcmp(tok->run, "the")) {
      reason = msgs.err.syn.op.rem_quot_1;
      synerr();
   }

   gettok();
   if (strcmp(tok->run, "quotient")) {
      reason = msgs.err.syn.op.rem_quot_2;
      synerr();
   }

   rem_quot = graft_tree(op, NULL, 0, "REM_QUOT");
   parse_op_quot(rem_quot);
}

static void parse_op_sqrt(tree_t *op) {
   /*
    * the square root of <const>
    */
   parse_op_unary(op, "of", msgs.err.syn.op.sqrt);
}

static void parse_op_squr(tree_t *op) {
   /*
    * the square of <const>
    */
   parse_op_unary(op, "of", msgs.err.syn.op.squr);
}

static void parse_op_cube(tree_t *op) {
   /*
    * the cube of <const>
    */
   parse_op_unary(op, "of", msgs.err.syn.op.cube);
}

static void parse_op_2x(tree_t *op) {
   parse_const(op);
}

static void parse_op_fact(tree_t *op) {
   /*
    * the factorial of <const>
    */
   parse_op_unary(op, "of", msgs.err.syn.op.fact);
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
   line = graft_tree(scene, NULL, 0, "LINE");
   character = graft_tree(line, NULL, 0, "CHAR");

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
         JUMP(STMTKIND_FINALE);
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
   if (strcmp(tok->run, "You") && strcmp(tok->run, "Thou"))
      return 0;
   return 1;
}

static void parse_asgn(void) {
   int type;
   bool cond1, cond2;

   if (!strcmp(tok->run, "You"))
      type = 1;
   else  /* Thou */
      type = 2;

   reason = msgs.err.syn.asgn.incomp;
   gettok();

   cond1 = !strcmp(tok->run, "are");
   cond2 = !strcmp(tok->run, "art");

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
   if (strcmp(tok->run, "as")) {
      reason = msgs.err.syn.asgn.no_as;
      synerr();
   }

   gettok();
   if (tok->kind == TOKKIND_PNT) {
      reason = msgs.err.syn.asgn.no_adj;
      synerr();
   }

   asgn_i = graft_tree(line, tok->run, tok->len, "ASSIGN");

   gettok();
   if (strcmp(tok->run, "as")) {
      reason = msgs.err.syn.asgn.no_as;
      synerr();
   }

   parse_const(asgn_i);
}

static void parse_asgn_ii(void) {
   tree_t *asgn_ii;

   asgn_ii = graft_tree(line, NULL, 0, "ASSIGN");
   parse_const(asgn_ii);
}

static void seek_stmt(void) {
   if (idx == len - 1)
      JUMP(STMTKIND_FINALE);

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
      { seek_line  , STMTKIND_LINE   },
      { seek_enter , STMTKIND_ENTER  },
      { seek_exit  , STMTKIND_EXIT   },
      { seek_exeunt, STMTKIND_EXEUNT },
      { seek_scene , STMTKIND_SCENE  },
      { seek_act   , STMTKIND_ACT    }
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

   ret = setjmp(env_parse_stmt);
   switch (ret) {
      case STMTKIND_ENTER  : parse_enter();  break;
      case STMTKIND_EXIT   : parse_exit();   break;
      case STMTKIND_EXEUNT : parse_exeunt(); break;
      case STMTKIND_LINE   : parse_line();   break; /* seek_stmt */
      case STMTKIND_SCENE  : /* fall-through */
      case STMTKIND_ACT    : /* fall-through */
      case STMTKIND_FINALE : return ret;
   }
   seek_stmt(); /* longjmp */

   return 0;  /* control never reaches here */
}

static int seek_out(void) {
   /*
    * Need to check the third token because of
    * the input statement "Open your mind".
    */
   if (!strcmp(tok->run, "Open")) {
      gettokn(2);
      if (strcmp(tok->run, "heart"))
         return 0;
      ungettokn(2);
      return 1;
   }
   else
   if (!strcmp(tok->run, "Speak"))
      return 1;
   else
      return 0;
}

static void parse_out(void) {
   tree_t *out;
   token_t *tok1, *tok2;
   const char *tag;
   bool mcond, lcond1, lcond2;
   int type;

   if (!strcmp(tok->run, "Open"))
      type = 1, tag = "OUT_NUM";
   else  /* Speak */
      type = 2, tag = "OUT_CHAR";

   reason = msgs.err.syn.out.incomp;
   out = graft_tree(line, NULL, 0, tag);

   gettok(); tok1 = tok;
   gettok(); tok2 = tok;

   mcond = !strcmp(tok1->run, "your")
      || !strcmp(tok1->run, "YOUR")
      || !strcmp(tok1->run, "thy");
   lcond1 = !strcmp(tok2->run, "heart");
   lcond2 = !strcmp(tok2->run, "mind");

   if (!mcond) {
      reason = msgs.err.syn.out.badsyn;
      etok = tok1;
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
   if (strcmp(tok->run, "Listen") && strcmp(tok->run, "Open"))
      return 0;
   else
      return 1;
}

static void parse_in(void) {
   tree_t *in;
   token_t *tok1, *tok2;
   const char *tag;
   bool mcond, lcond1, lcond2;
   int type;

   if (!strcmp(tok->run, "Listen"))
      type = 1, tag = "IN_NUM";
   else  /* Open */
      type = 2, tag = "IN_CHAR";

   reason = msgs.err.syn.in.incomp;
   in = graft_tree(line, NULL, 0, tag);

   if (type == 1) {
      gettok();
      if (strcmp(tok->run, "to")) {
         reason = msgs.err.syn.in.badsyn;
         synerr();
      }
   }

   gettok(); tok1 = tok;
   gettok(); tok2 = tok;

   mcond = !strcmp(tok1->run, "your") || !strcmp(tok1->run, "YOUR");
   lcond1 = !strcmp(tok2->run, "heart");
   lcond2 = !strcmp(tok2->run, "mind");

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
   if (strcmp(tok->run, "Let") && strcmp(tok->run, "We"))
      return 0;
   return 1;
}

static void parse_goto(void) {
   tree_t *gt;
   int type;
   bool cond1, cond2;

   if (!strcmp(tok->run, "Let"))
      type = 1;
   else  /* we */
      type = 2;

   reason = msgs.err.syn.gt.incomp;
   gettok();
   cond1 = !strcmp(tok->run, "us");
   cond2 = !strcmp(tok->run, "shall") || !strcmp(tok->run, "must");

   if (!cond1 && !cond2) {
      reason = msgs.err.syn.gt.badsyn;
      synerr();
   }
   if ((type == 1 && cond2) || (type == 2 && cond1)) {
      reason = msgs.err.syn.gt.unmatched;
      synerr();
   }

   gettok();
   cond1 = strcmp(tok->run, "return");
   cond2 = strcmp(tok->run, "proceed");

   if (cond1 && cond2) {
      reason = msgs.err.syn.gt.badsyn;
      synerr();
   }

   gettok();
   if (strcmp(tok->run, "to")) {
      reason = msgs.err.syn.gt.badsyn;
      synerr();
   }

   gettok();
   if (strcmp(tok->run, "Scene")) {
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

   (void) graft_tree(line, tok->run, tok->len, "GOTO");

   gettok();
   if (!match(tok->run[0], ".!")) {
      reason = msgs.err.syn.in.badsyn;
      synerr();
   }
}

static int seek_cond(void) {
   bool cond1, cond2, cond3, cond4;

   cond1 = strcmp(tok->run, "Am");
   cond2 = strcmp(tok->run, "Are");
   cond3 = strcmp(tok->run, "Art");
   cond4 = strcmp(tok->run, "Is");

   if (cond1 && cond2 && cond3 && cond4)
      return 0;
   else
      return 1;
}

static void parse_cond(void) {
   tree_t *condition, *lefthand, *righthand;
   const char *person, *truthval;
   bool cond1, cond2, cond3;
   int type;

   reason = msgs.err.syn.cond.incomp;
   condition = graft_tree(line, NULL, 0, "QUESTION");

   if (!strcmp(tok->run, "Am"))
      type = 1, person = "first-person";
   else
   if (!strcmp(tok->run, "Are"))
      type = 2, person = "second-person";
   else
   if (!strcmp(tok->run, "Art"))
      type = 3, person = "second-person";
   else
   if (!strcmp(tok->run, "Is"))
      type = 4, person = "third-person";

   if (type < 4) {
      gettok();
      cond1 = !strcmp(tok->run, "I");
      cond2 = !strcmp(tok->run, "you");
      cond3 = !strcmp(tok->run, "thou");

      cond1 = type == 1 && !cond1;
      cond2 = type == 2 && !cond2;
      cond3 = type == 3 && !cond3;

      if (cond1 || cond2 || cond3) {
         reason = msgs.err.syn.cond.unmatched;
         synerr();
      }
   }
   else parse_const(condition);

   lefthand  = graft_tree(condition, NULL, 0, "LHS");
   (void) graft_tree(
      lefthand, person, strlen(person) + 1, "PERSON");

   gettok();
   if (!strcmp(tok->run, "not")) {
      truthval = "true";
      gettok();
   }
   else truthval = "false";

   righthand = graft_tree(condition, NULL, 0, "RHS");
   (void) graft_tree(
      condition, truthval, strlen(truthval) + 1, "NOT");

   if (!strcmp(tok->run, "as")) {
      gettok();
      if (tok->kind == TOKKIND_PNT) {
         reason = msgs.err.syn.cond.badsyn;
         synerr();
      }

      (void) graft_tree(
         condition, tok->run, tok->len, "EQUAL");

      gettok();
      if (strcmp(tok->run, "as")) {
         reason = msgs.err.syn.cond.badsyn;
         synerr();
      }
   }
   else {
      (void) graft_tree(
         condition, tok->run, tok->len, "COMPARE");

      gettok();
      if (type < 4 && strcmp(tok->run, "than")) {
         reason = msgs.err.syn.cond.badsyn;
         synerr();
      }
   }

   parse_const(righthand);
}

static int seek_if(void) {
   return strcmp(tok->run, "If") ? 0 : 1;
}

static void parse_if(void) {
   tree_t *ifstmt, *consequent, *tline;
   const char *truthval;
   int ret;

   reason = msgs.err.syn.ifstmt.incomp;
   gettok();

   if (!strcmp(tok->run, "so"))
      truthval = "true";
   else
   if (!strcmp(tok->run, "not"))
      truthval = "false";
   else {
      reason = msgs.err.syn.ifstmt.badsyn;
      synerr();
   }

   ifstmt = graft_tree(line, NULL, 0, "IF");
   (void) graft_tree(ifstmt, truthval, strlen(truthval) + 1, "ANT");
   consequent = graft_tree(ifstmt, NULL, 0, "CON");

   gettok();
   if (tok->run[0] != ',') {
      reason = msgs.err.syn.ifstmt.badsyn;
      synerr();
   }

   // Parse the consequent
   tline = line;
   line = consequent;

   reason = msgs.err.syn.ifstmt.conseq_incomp;
   ret = parse_line_as_conseq();

   if (ret == 1) {
      reason = msgs.err.syn.ifstmt.bad_conseq;
      synerr();
   }
   else
   if (ret == 2) {
      reason = msgs.err.syn.ifstmt.conseq_cap;
      synerr();
   }

   line = tline;
}

static int seek_push(void) {
   return strcmp(tok->run, "Remember") ? 0 : 1;
}

static void parse_push(void) {
   reason = msgs.err.syn.push.incomp;
   gettok();

   if (tok->kind == TOKKIND_PNT) {
      reason = msgs.err.syn.push.badsyn;
      synerr();
   }

   (void) graft_tree(line, tok->run, tok->len, "PUSH");

   gettok();
   if (!match(tok->run[0], ".!")) {
      reason = msgs.err.syn.in.badsyn;
      synerr();
   }
}

static int seek_pop(void) {
   return strcmp(tok->run, "Recall") ? 0 : 1;
}

static void parse_pop(void) {
   reason = msgs.err.syn.pop.incomp;
   (void) graft_tree(line, NULL, 0, "POP");
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
         base, tok->run, tok->len, "CONTENT");
   }
   etok = tok;
}

static int match_tokrun(const char **arr, int len) {
   for (int i = 0; i < len; i++)
      if (!strcmp(tok->run, arr[i]))
         return i;
   return -1;
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
   l = arr_peek(ls, etok->lnum - 1);

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
   const char * restrict run,
   int len,
   const char * restrict tag,
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
   strncpy(node.tag, tag, NODE_TAG_LEN + 1);
   node.lnum = lnum;
   node.lpos = lpos;

   return tree_plant(&node, sizeof node);
}

static tree_t *graft_tree(
   tree_t *base,
   const char * restrict run,
   int len,
   const char * restrict tag
) {
   tree_t *ret, *sub;

   /*
    * Need to set `reason` variable before
    * `graft_tree` call.
    */
   gettok();
   sub = plant_tree(
      run,
      len,
      tag,
      tok->lnum,
      tok->lpos
   );
   ret = tree_graft(base, sub);
   ungettok();

   return ret;
}
