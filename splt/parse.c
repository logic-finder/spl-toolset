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

/* Parse Tree */
extern tree_t
   *pt;     // parse tree (see global.h)
static tree_t
   *nrtv,   // contains the whole narrative
   *act,    // current act
   *scene,  // current scene
   *line;   // current line

/* Miscellaneous */
extern const char *sfname;   // see global.h
extern msg_t msgs;           // see global.h

static const char *reason;   // error message
static jmp_buf LONGJMP_ENV;  // for setjmp & longjmp
static int charidx;          // used by `isname` & its caller

/*
 * `seek_if` and `parse_if` being the
 * first element is intentional;
 * refer to `parse_line_as_conseq`.
 */
static const stmthandler_t stmts[] = {
   { seek_asgn , parse_asgn },
   { seek_out  , parse_out  },
   { seek_in   , parse_in   },
   { seek_goto , parse_goto },
   { seek_cond , parse_cond },
   { seek_if   , parse_if   },
   { seek_push , parse_push },
   { seek_pop  , parse_pop  }
};
static const int stmts_len = ARRLEN(stmts);

/* These are used in `seek_cond` and `parse_cond` only. */
static const char *cond_verbs[] = {
   KEYWRD_AM, KEYWRD_ARE_C, KEYWRD_ART_C, KEYWRD_IS
};
static const int cond_verbs_len = ARRLEN(cond_verbs);
static const char *cond_subjs[] = {
   KEYWRD_I, KEYWRD_YOU_L, KEYWRD_THOU_L
};
static const int cond_subjs_len = ARRLEN(cond_subjs);
static int vtype;

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

   /*
    * Construct the parse tree
    * pt[0] = title
    * pt[1] = dp
    * pt[2] = nrtv
    */
   act = scene = line = NULL;
   parse_title();
   parse_dp();
   nrtv = graft_tree_n(pt, 0, NODEKIND_NRTV);
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
   title = graft_tree_n(pt, 0, NODEKIND_TITLE);
   readtoks('.', title);
}

static void parse_dp(void) {
   tree_t *dp, *character;

   dp = graft_tree_n(pt, 0, NODEKIND_DP);
   for (;;) {
      reason = msgs.err.syn.dp.incomp;
      gettok();
      reason = msgs.err.syn.dp.noname;
      eqtok(',');
      character = graft_tree_n(dp, 0, NODEKIND_CHDECL);
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
   act = graft_tree_n(nrtv, 0, NODEKIND_ACT);
   reason = msgs.err.syn.act.incomp;
   gettok();
   if (tok->kind == TOKKIND_PNT) {
      reason = msgs.err.syn.act.nornum;
      synerr();
   }
   (void) graft_tree_s(act, tok->run, tok->len, NODEKIND_ROMNUM);
   gettok();
   reason = msgs.err.syn.act.nocolon;
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
   scene = graft_tree_n(act, 0, NODEKIND_SCENE);
   reason = msgs.err.syn.scene.incomp;
   gettok();
   if (tok->kind == TOKKIND_PNT) {
      reason = msgs.err.syn.scene.nornum;
      synerr();
   }
   (void) graft_tree_s(scene, tok->run, tok->len, NODEKIND_ROMNUM);
   gettok();
   reason = msgs.err.syn.scene.nocolon;
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

static void parse_namelist(tree_t *enterlike, const char *err) {
   for (;;) {
      gettok();
      archive_tokstate();  /* `isname` rewinds tokstate */
      if (tok->run[0] == ']')
         break;
      if (!isname_lower()) {
         reason = err;
         synerr();
      }
      (void) graft_tree_n(enterlike, charidx, NODEKIND_CHAR);
      if (tok->run[0] == ']')
         break;
      if (!strcmp(tok->run, KEYWRD_AND))
         continue;
   }
}

static void parse_enter(void) {
   tree_t *enter;

   enter = graft_tree_n(scene, 0, NODEKIND_ENTER);
   reason = msgs.err.syn.enter.incomp;

   // Enter has 1 or 2 characters
   parse_namelist(enter, msgs.err.syn.enter.badname);

   switch (tree_clen(enter)) {
      case 1 : /* fall-through */
      case 2 : return;
      case 0  : reason = msgs.err.syn.enter.nochar; break;
      default : reason = msgs.err.syn.enter.exceed;
   }
}

static int seek_exit(void) {
   return seek_enterlike(KEYWRD_EXIT);
}

static void parse_exit(void) {
   tree_t *exit;

   exit = graft_tree_n(scene, 0, NODEKIND_EXIT);
   reason = msgs.err.syn.exit.incomp;

   // Exit has 1 character
   parse_namelist(exit, msgs.err.syn.exit.badname);

   switch (tree_clen(exit)) {
      case 1 : return;
      case 0  : reason = msgs.err.syn.exit.nochar; break;
      default : reason = msgs.err.syn.exit.exceed;
   }
   synerr();
}

static int seek_exeunt(void) {
   return seek_enterlike(KEYWRD_EXEUNT);
}

static void parse_exeunt(void) {
   tree_t *exeunt;

   exeunt = graft_tree_n(scene, 0, NODEKIND_EXEUNT);
   reason = msgs.err.syn.exeunt.incomp;

   // Exeunt has either 0 or 2 characters
   parse_namelist(exeunt, msgs.err.syn.exeunt.badname);

   switch (tree_clen(exeunt)) {
      case 0 : /* fall-through  */
      case 2 : return;
      case 1  : reason = msgs.err.syn.exeunt.onechar; break;
      default : reason = msgs.err.syn.exeunt.exceed;
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

   tree_t *constant;
   bool opcond, cond1, cond2, cond3, cond4, cond5, cond6;
   int ret;
   nodekind_t kind;

   // Make a constant node
   constant = graft_tree_n(stmt, 0, NODEKIND_CONST);
   reason = msgs.err.syn.cnst.incomp;
   gettok();

   /*
    * Since names can begin with 'the', we first check
    * whether this constant is a name or not.
    */
   archive_tokstate();  /* `isname` rewinds tokstate */
   ret = isname_lower();
   if (ret)
      goto name;

   /*
    * If this constant begins with a possessive or
    * an article, skip it.
    */
   ret = match_str(tok->run, decos, decos_len);
   if (ret >= 0)
      gettok();

   // Checkes if this token is an operator
   opcond = (kind = seek_op()) != NODEKIND__NAO;
   if (opcond) {
      parse_op(constant, kind);
      return;
   }

   /*
    * Processes a series of tokens as a constant
    * and stops if one of the escape conditions
    * is met.
    */
   for (;;) {
      // escape conditions
      cond1 = match(tok->run[0], ".!?");
      if (cond1) goto noun;
      cond2 = !strcmp(tok->run, KEYWRD_AND);
      if (cond2) goto noun;
      cond3 = !strcmp(tok->run, KEYWRD_AS);
      if (cond3) goto noun;
      cond4 = !strcmp(tok->run, KEYWRD_NOT);
      if (cond4) goto noun;
      cond5 = !strcmp(tok->run, KEYWRD_THAN);
      if (cond5) { ungettokn(2); goto noun; }
      archive_tokstate();  /* `isname` rewinds tokstate */
      cond6 = isname_lower();
      if (cond6) goto name;

      // is it a decorator?
      if (match_str(tok->run, decos, decos_len) >= 0) {
         reason = msgs.err.syn.cnst.deco;
         synerr();
      }

      (void) graft_tree_s(constant, tok->run, tok->len, NODEKIND_ADJ);

      reason = msgs.err.syn.cnst.incomp;
      gettok();
   }

   noun:
      TREE_CHDAT(constant, tree_clen(constant) - 1)->kind = NODEKIND_NOUN;
   return;

   name:
      (void) graft_tree_n(constant, charidx, NODEKIND_CHAR);
   return;
}

static void parse_cond_eq(tree_t *cond) {
   gettok();
   if (tok->kind == TOKKIND_PNT) {
      reason = msgs.err.syn.cond.badsyn;
      synerr();
   }

   (void) graft_tree_s(
      cond, tok->run, tok->len, NODEKIND_EQ);

   gettok();
   if (strcmp(tok->run, KEYWRD_AS)) {
      reason = msgs.err.syn.cond.badsyn;
      synerr();
   }
}

static void parse_cond_ineq(tree_t *cond) {
   static const char *comps[2] = { "more", "less" };
   static const int comps_len = ARRLEN(comps);

   int ret;
   nodekind_t kind;

   ret = match_str(tok->run, comps, comps_len);

   if (ret > 0) {
      gettok();
      if (tok->kind == TOKKIND_PNT) {
         reason = msgs.err.syn.cond.badsyn;
         synerr();
      }
      switch (ret) {
         case 0 : kind = NODEKIND_GT; break;
         case 1 : kind = NODEKIND_LT; break;
      }
      (void) graft_tree_s(cond, tok->run, tok->len, kind);
   }
   else {
      (void) graft_tree_s(
         cond, tok->run, tok->len, NODEKIND_INEQ);
      gettok();
      if (strcmp(tok->run, KEYWRD_THAN)) {
         reason = msgs.err.syn.cond.badsyn;
         synerr();
      }
   }
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
   static const int ops_len = ARRLEN(ops);

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

   op = graft_tree_n(stmt, 0, kind);
   reason = msgs.err.syn.op.incomp;
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

   lefthand = graft_tree_n(op, 0, NODEKIND_LHS);
   parse_const(lefthand);
   if (strcmp(tok->run, KEYWRD_AND)) {
      reason = msgs.err.syn.op.no_and;
      synerr();
   }
   righthand = graft_tree_n(op, 0, NODEKIND_RHS);
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

static int isname(void) {
   tree_t *dp, *character;
   int i, k, dp_clen, char_clen;

   dp = tree_child(pt, 1);
   dp_clen = tree_clen(dp);

   for (i = 0; i < dp_clen; i++) {
      character = tree_child(dp, i);
      char_clen = tree_clen(character);
      for (k = 0; k < char_clen; k++) {
         if (strcmp(TREE_CHDAT(character, k)->dat.s.run, tok->run)) {
            rewind_tokstate();
            goto next;
         }
         gettok();
      }
      charidx = i;
      return 1;
      next:;
   }
   return 0;
}

static int isname_lower(void) {
   if (!strcmp(tok->run, "A")
      || !strcmp(tok->run, "An")
      || !strcmp(tok->run, "The")
   ) {
      reason = msgs.err.syn.name_not_lowcase;
      synerr();
   }

   if (!strcmp(tok->run, KEYWRD_A)
      || !strcmp(tok->run, KEYWRD_AN)
      || !strcmp(tok->run, KEYWRD_THE)
   ) tok->run[0] = toupper(tok->run[0]);

   if(isname())
      return 1;
   tok->run[0] = tolower(tok->run[0]);
   return 0;
}

static int seek_line(void) {
   return isname();
}

static void parse_line(void) {
   // Make nodes (charidx was updated by isname() in seek_line)
   line = graft_tree_n(scene, 0, NODEKIND_LINE);
   (void) graft_tree_n(line, charidx, NODEKIND_CHAR);

   // Handle the first statement
   reason = msgs.err.syn.line.incomp;
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

   return parse_line_router(stmts, stmts_len);
}

static int parse_line_router(
   const stmthandler_t stmts[8],
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
   if (strcmp(tok->run, KEYWRD_YOU)
      && strcmp(tok->run, KEYWRD_THOU)
   ) return 0;
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

   asgn_i = graft_tree_s(line, tok->run, tok->len, NODEKIND_ASGN1);

   gettok();
   if (strcmp(tok->run, KEYWRD_AS)) {
      reason = msgs.err.syn.asgn.no_as;
      synerr();
   }

   parse_const(asgn_i);
}

static void parse_asgn_ii(void) {
   tree_t *asgn_ii;

   asgn_ii = graft_tree_n(line, 0, NODEKIND_ASGN2);
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
      { seek_line   , NODEKIND_LINE   },
      { seek_enter  , NODEKIND_ENTER  },
      { seek_exit   , NODEKIND_EXIT   },
      { seek_scene  , NODEKIND_SCENE  },
      { seek_act    , NODEKIND_ACT    },
      { seek_exeunt , NODEKIND_EXEUNT }
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
      case NODEKIND_ENTER  : parse_enter (); break;
      case NODEKIND_EXIT   : parse_exit  (); break;
      case NODEKIND_EXEUNT : parse_exeunt(); break;
      case NODEKIND_LINE   : parse_line  (); break; /* seek_stmt */
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
    * Need to check the third token so as to
    * differentiate it from the input statement
    * "Open your mind".
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

   (void) graft_tree_n(line, 0, kind);
   reason = msgs.err.syn.out.incomp;
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
    * This function utilizes the fact that `seek_out`
    * is executed first. Refer to `seek_stmt` and
    * stmts[]. That is, it doesn't check the third
    * token as in `seek_out`.
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
   (void) graft_tree_n(line, 0, kind);

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
   int type, mark;
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
   if (!strcmp(tok->run, KEYWRD_ACT))
      mark = NODEKIND_ACT;
   else
   if (!strcmp(tok->run, KEYWRD_SCENE))
      mark = NODEKIND_SCENE;
   else {
      if (!strcmp(tok->run, "act"))
         reason = msgs.err.syn.gt.act_misspell;
      else
      if (!strcmp(tok->run, "scene"))
         reason = msgs.err.syn.gt.scene_misspell;
      else
         reason = msgs.err.syn.gt.badsyn;
      synerr();
   }

   gt = graft_tree_n(line, mark, NODEKIND_GOTO);

   gettok();
   if (tok->kind == TOKKIND_PNT) {
      reason = msgs.err.syn.gt.badsyn;
      synerr();
   }

   (void) graft_tree_s(gt, tok->run, tok->len, NODEKIND_ROMNUM);

   gettok();
   if (!match(tok->run[0], ".!")) {
      reason = msgs.err.syn.gt.badsyn;
      synerr();
   }
}

static int seek_cond(void) {
   vtype = match_str(tok->run, cond_verbs, cond_verbs_len);
   if (vtype < 0)
      return 0;
   else
      return 1;
}

static void parse_cond(void) {
   tree_t *condition,
          *lefthand,
          *righthand,
          *p;
   nodekind_t kind;

   condition = graft_tree_n(line, 0, NODEKIND_COND);
   lefthand  = graft_tree_n(condition, 0, NODEKIND_LHS);

   reason = msgs.err.syn.cond.incomp;
   gettok();
   switch (vtype) {
      case 0 : if (strcmp(tok->run, KEYWRD_I))
                  goto hell; else break;
      case 1 : if (strcmp(tok->run, KEYWRD_YOU_L))
                  goto hell; else break;
      case 2 : if (strcmp(tok->run, KEYWRD_THOU_L))
                  goto hell; else break;
      case 3 : if (0 <= match_str(tok->run, cond_subjs, cond_subjs_len))
                  goto hell; else break;
      hell : /* FLAMING HOT */
         reason = msgs.err.syn.cond.not_conj;
         synerr();
   }
   if (vtype == 3) ungettok();

   switch (vtype) {
      case 0 : kind = NODEKIND_P1; break;
      case 1 : /* fall-through */
      case 2 : kind = NODEKIND_P2; break;
      case 3 : kind = NODEKIND_P3; break;
   }
   p = graft_tree_n(lefthand, 0, kind);

   if (vtype == 3)
      parse_const(p);

   gettok();
   if (!strcmp(tok->run, KEYWRD_NOT)) {
      (void) graft_tree_n(condition, 0, NODEKIND_NEGATE);
      gettok();
   }
   else
      (void) graft_tree_n(condition, 0, NODEKIND_AFFIRM);

   if (!strcmp(tok->run, KEYWRD_AS))
      parse_cond_eq(condition);
   else
      parse_cond_ineq(condition);

   righthand = graft_tree_n(condition, 0, NODEKIND_RHS);
   parse_const(righthand);
}

static int seek_if(void) {
   return strcmp(tok->run, KEYWRD_IF) ? 0 : 1;
}

static void parse_if(void) {
   tree_t *ifstmt, *consequent, *tline;
   int ret;

   ifstmt = graft_tree_n(line, 0, NODEKIND_IF);

   reason = msgs.err.syn.ifstmt.incomp;
   gettok();
   if (!strcmp(tok->run, KEYWRD_SO))
      (void) graft_tree_n(ifstmt, 0, NODEKIND_AFFIRM);
   else
   if (!strcmp(tok->run, KEYWRD_NOT))
      (void) graft_tree_n(ifstmt, 0, NODEKIND_NEGATE);
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
   consequent = graft_tree_n(ifstmt, 0, NODEKIND_CONSEQ);
   tline = line;
   line = consequent;
   reason = msgs.err.syn.ifstmt.conseq_incomp;
   ret = parse_line_as_conseq();
   line = tline;

   switch (ret) {
      case 0 : return;
      case 1 :
         reason = msgs.err.syn.ifstmt.bad_conseq;
         goto error;
      case 2 :
         reason = msgs.err.syn.ifstmt.conseq_cap;
         goto error;
      error : synerr();
   }
}

static int seek_push(void) {
   return strcmp(tok->run, KEYWRD_REMEMB) ? 0 : 1;
}

static void parse_push(void) {
   tree_t *push;

   push = graft_tree_n(line, 0, NODEKIND_PUSH);

   reason = msgs.err.syn.push.incomp;
   gettok();
   if (tok->kind == TOKKIND_PNT) {
      reason = msgs.err.syn.push.badsyn;
      synerr();
   }

   parse_const(push);

   gettok();
   if (!match(tok->run[0], ".!")) {
      reason = msgs.err.syn.push.badsyn;
      synerr();
   }
}

static int seek_pop(void) {
   return strcmp(tok->run, KEYWRD_RECALL) ? 0 : 1;
}

static void parse_pop(void) {
   (void) graft_tree_n(line, 0, NODEKIND_POP);
   reason = msgs.err.syn.pop.incomp;
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
      (void) graft_tree_s(
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

static void synerr(void) {
   int lnum, lpos;
   line_t *l;

   lnum = etok->lnum;
   lpos = etok->lpos;
   l = arr_peek(ls, lnum - 1);

   fmtwrt(
      Cbred "\n<syntax error>" Creset " %s\n"
      "[%s:%d:%d] " Cbwhite "note:" Creset " problematic since here\n"
      "%4d|%.*s" Cbblue "%s" Creset "\n",
      reason,
      sfname, lnum, lpos,
      lnum, lpos - 1, l->run, &l->run[lpos - 1]
   );
   exit(EXIT_FAILURE);
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
   const char *run,
   int len,
   nodekind_t kind
) {
   tree_t *sub;

   sub = plant_tree(
      run,
      len,
      kind,
      tok->lnum,
      tok->lpos
   );

   return tree_graft(base, sub);
}

static tree_t *graft_tree_n(
   tree_t *base,
   int num,
   nodekind_t kind
) {
   node_t node;
   tree_t *sub;

   node.datkind = DATKIND_INT;
   node.dat.n = num;
   node.kind = kind;
   node.lnum = tok->lnum;
   node.lpos = tok->lpos;
   sub = tree_plant(&node, sizeof node);

   return tree_graft(base, sub);
}

void setndn(node_t *n, int v) {
   n->datkind = DATKIND_INT;
   n->dat.n = v;
}

void setnds(node_t *n, char *s, int l) {
   n->datkind = DATKIND_STR;
   n->dat.s.run = s;
   n->dat.s.len = l;
}
