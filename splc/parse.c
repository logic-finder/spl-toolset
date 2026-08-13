#include "parse.h"
#include "parse.internals.h"

// fixme: STREQL 매크로 정의해서 쓰기 (!strcmp 대체 프로젝트 전체적으로)

extern tree_t *parse(optflg_t *of, optval_t *ov, array_t *tokens) {
   (void) of, (void) ov;

   /* Initializes global variables */
   toks = tokens;
   tok = array_peek(toks, 0);
   len = array_size(toks);
   idx = -1;
   pt = plant_tree(NULL, 0, NODEKIND_ROOT, 0, 0);

   /* Constructs the parse tree
         pt[0] = title
         pt[1] = dp
         pt[2] = nrtv  */

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

   /* Cleanup */
   array_foreach(toks, cleanup_tokstream);
   array_destroy(toks);

   return pt;
}

static void cleanup_tokstream(void *tok, int idx) {
   (void) idx;
   free(((token_t *) tok)->run);
}

static void parse_title(void) {
   tree_t *title;

   reason = msgs.err.syn.title.incomp;
   title = graft_tree_n(pt, 0, NODEKIND_TITLE);
   readtoks_until(".!?", title);
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
      skiptoks2(".!?");
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
   // fixme: 실제로 로마 숫자인지 검사할 것
   if (tok->kind == TOKKIND_PNT) {
      reason = msgs.err.syn.act.nornum;
      synerr();
   }
   (void) graft_tree_s(act, tok->run, tok->len, NODEKIND_ROMNUM);
   gettok();
   reason = msgs.err.syn.act.nocolon;
   neqtok(':');
   reason = msgs.err.syn.act.desc_incomp;
   skiptoks2(".!?");
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
   // fixme: 실제로 로마 숫자인지 검사할 것
   if (tok->kind == TOKKIND_PNT) {
      reason = msgs.err.syn.scene.nornum;
      synerr();
   }
   (void) graft_tree_s(scene, tok->run, tok->len, NODEKIND_ROMNUM);
   gettok();
   reason = msgs.err.syn.scene.nocolon;
   neqtok(':');
   reason = msgs.err.syn.scene.desc_incomp;
   skiptoks2(".!?");
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
   tree_t *enter;

   enter = graft_tree_n(scene, 0, NODEKIND_ENTER);
   parse_namelist(enter);
   if (tree_clen(enter))
      return;
   reason = msgs.err.syn.enter.nochar;
   synerr();
}

static int seek_exit(void) {
   return seek_enterlike(KEYWRD_EXIT);
}

static void parse_exit(void) {
   tree_t *exit;

   exit = graft_tree_n(scene, 0, NODEKIND_EXIT);
   parse_namelist(exit);
   switch (tree_clen(exit)) {
      case 0  : reason = msgs.err.syn.exit.nochar; break;
      case 1  : return;
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
   parse_namelist(exeunt);
   switch (tree_clen(exeunt)) {
      case 0  : return;
      case 1  : reason = msgs.err.syn.exeunt.onechar; break;
      case 2  : return;
      default : reason = msgs.err.syn.exeunt.exceed;
   }
   synerr();
}

static void parse_namelist(tree_t *t) {
   /* Enter, Exit, and Exeunt takes a namelist:
         (1) [<enterlike> A]
         (2) [<enterlike> A and B]
         (3) [<enterlike> A, B, and C] */

   reason = "incomplete namelist";
   gettok();
   if (tok->run[0] == ']')
         return;
   ungettok();

   /* Currently, tok->run points to <enterlike> */

   /* Consumes tokens until "and" */
   do {
      reason = "incomplete namelist"; // fixme: remove this
      gettok();
      if (!is_name_lower()) {
         reason = "dp expected here";
         synerr();
      }
      graft_tree_n(t, charidx, NODEKIND_CHAR);
      gettok();
      if (tok->run[0] == ']')
         return;
      if (!strcmp(tok->run, "and"))
         break;
      if (tok->run[0] != ',') {
         reason = ", expected here";
         synerr();
      }
      gettok();
      if (!strcmp(tok->run, "and"))
         break;
      ungettok();
   } while (true);

   /* Skips "and" and consumes the last name */
   gettok();
   if (!is_name_lower()) {
      reason = "dp expected here";
      synerr();
   }
   graft_tree_n(t, charidx, NODEKIND_CHAR);
   gettok();
   if (tok->run[0] != ']') {
      reason = "] expected here";
      synerr();
   }
}

static void parse_const(tree_t *stmt) {
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
   cnst = graft_tree_n(stmt, 0, NODEKIND_CONST);

   reason = msgs.err.syn.cnst.incomp;
   gettok();

   /* First of all, we check whether this is an operator */
   if ((kind = seek_op()) != NODEKIND__NAO) {
      parse_op(cnst, kind);
      return;
   }

   /* Checks whether this token is
      a pronoun, a reflexive, a name, or a nil */
   if (is_pronoun(tok->run)) {
      graft_tree_s(cnst, tok->run, tok->len, what_pronoun(tok->run));
      check_const_end();
      return;
   }
   if (is_reflexive(tok->run)) {
      graft_tree_s(cnst, tok->run, tok->len, what_reflexive(tok->run));
      check_const_end();
      return;
   }
   if (is_name_lower()) {
      graft_tree_n(cnst, charidx, NODEKIND_CHAR);
      check_const_end();
      return;
   }
   if (is_nil(tok->run)) {
      graft_tree_s(cnst, tok->run, tok->len, NODEKIND_ZERO);
      check_const_end();
      return;
   }

   /* If not, this token is either TYPE A or TYPE B. Meanwhile,
      TYPE B = (art|pos) TYPE A. Let's exploit this structure */
   if (is_article(tok->run) || is_possessive(tok->run)) {
      reason = msgs.err.syn.cnst.incomp;
      gettok();  /* skips the current token */
      /* if the token had been of TYPE B, now it has become of TYPE A */
   }

   /* Now we need to process <adj|ap> (noun|np) */

   /* Consumes adjectives first */
   for (;;) {
      if (!query_adj(tok->run))
         break;
      graft_tree_s(cnst, tok->run, tok->len, NODEKIND_ADJ);
      gettok();
   }

   /* noun? */
   if (query_noun(tok->run, &query_result)) {
      kind = query_result ? NODEKIND_PNOUN : NODEKIND_NNOUN;
      graft_tree_s(cnst, tok->run, tok->len, kind);
   }
   else {  /* noun phrase */
      // TODO: refactor later!!
      token_t *prev_tok;
      char *buf;
      size_t bufsiz;

      prev_tok = tok;

      reason = msgs.err.syn.cnst.incomp;
      gettok();

      bufsiz = prev_tok->len + tok->len;
      buf = safe_malloc(bufsiz);
      memcpy(buf, prev_tok->run, prev_tok->len);
      buf[prev_tok->len - 1] = ' ';
      memcpy(buf + prev_tok->len, tok->run, tok->len);

      if(!query_noun(buf, &query_result)) {
         ungettok();
         reason = msgs.err.syn.cnst.no_noun;
         synerr();
      }

      kind = query_result ? NODEKIND_PNOUN : NODEKIND_NNOUN;
      graft_tree_s(cnst, buf, bufsiz, kind);
   }

   /* end of const */
   check_const_end();
}

// TODO: refactor is_xxx: (1) use macro (2) use lowercase
static bool is_pronoun(const char *str) {
   static const char *pronouns[] = {
      "I", "me", "thee", "thou", "you", NULL
   };

   for (size_t i = 0; pronouns[i]; i++)
      if (!strcmp(str, pronouns[i]))
         return true;
   return false;
}

static bool is_reflexive(const char *str) {
   static const char *reflexives[] = {
      "myself", "thyself", "yourself", NULL
   };

   for (size_t i = 0; reflexives[i]; i++)
      if (!strcmp(str, reflexives[i]))
         return true;
   return false;
}

static bool is_nil(const char *str) {
   static const char *nils[] = {
      "nothing", "zero", NULL
   };

   for (size_t i = 0; nils[i]; i++)
      if (!strcmp(str, nils[i]))
         return true;
   return false;
}

static bool is_article(const char *str) {
   static const char *articles[] = {
      "a", "an", "the", NULL
   };

   for (size_t i = 0; articles[i]; i++)
      if (!strcmp(str, articles[i]))
         return true;
   return false;
}

static bool is_possessive(const char *str) {
   static const char *possessives[] = {
      "mine", "my", "thine", "thy", "your", "his", "her", "its", "theirs", NULL
   };

   for (size_t i = 0; possessives[i]; i++)
      if (!strcmp(str, possessives[i]))
         return true;
   return false;
}

static nodekind_t what_pronoun(const char *str) {
   if (!strcmp(str, "I") || !strcmp(str, "me"))
      return NODEKIND_P1;

   if (!strcmp(str, "thee")
      || !strcmp(str, "thou")
      || !strcmp(str, "you"))
      return NODEKIND_P2;

   /* control never reaches here */
   return NODEKIND__UNKNOWN;
}

static nodekind_t what_reflexive(const char *str) {
   if (!strcmp(str, "myself"))
      return NODEKIND_P1;

   if (!strcmp(str, "thyself") || !strcmp(str, "yourself"))
      return NODEKIND_P2;

   /* control never reaches here */
   return NODEKIND__UNKNOWN;
}

static void check_const_end(void) {
   reason = msgs.err.syn.cnst.incomp;
   gettok();

   if (strchr(".!?", tok->run[0])
      || !strcmp(tok->run, "not")
      || !strcmp(tok->run, "as")
      || !strcmp(tok->run, "more")
      || !strcmp(tok->run, "less")
      || !strcmp(tok->run, "and")
      || query_comp(tok->run, NULL))
      return;

   reason = msgs.err.syn.cnst.no_end_symbol;
   synerr();
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
   if (!strcmp(tok->run, "twice"))
      return NODEKIND_2X;

   /* not begins with "the"? then it's not an operator */
   if (strcmp(tok->run, "the"))
      return NODEKIND__NAO;

   reason = msgs.err.syn.cnst.incomp;
   gettok();

   k = NODEKIND__NAO;
   for (size_t i = 0; i < ops_len; i++)
      if (!strcmp(tok->run, ops[i].name)) {
         k = ops[i].kind;
         break;
      }

   /* no match! turns out it isn't an operator,
      although it began with "the" */
   if (k == NODEKIND__NAO) {
      ungettok();
      return k;
   }

   /* not a square or a square root operator */
   if (k != NODEKIND_SQUR)
      return k;

   /* to be a square or to be a square root? that's the question */
   gettok();
   if (!strcmp(tok->run, KEYWRD_ROOT))
      k = NODEKIND_SQRT;
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
      default: ;  /* control never reaches default */
   }
}

static void parse_op_unary(
   tree_t * restrict op,
   const char * restrict err
) {
   /* Note. this function is a wrapper for the
      square, square root, cube, and factorial.

      Since the twice operator does not have the
      same structure with other unary operators,
      it doesn't use this wrapper. */

   reason = msgs.err.syn.op.incomp;
   gettok();

   if (strcmp(tok->run, "of")) {
      reason = err;
      synerr();
   }

   parse_const(op);
}

static void parse_op_binary(
   tree_t * restrict op,
   const char * restrict type,
   const char * restrict err
) {
   tree_t *lefthand, *righthand;

   reason = msgs.err.syn.op.incomp;
   gettok();

   /* the sum OF L and R
      the difference BETWEEN L and R
      the product OF L and R
      the quotient BETWEEN L and R
      the remainder OF the quotient BETWEEN L and R */

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

static inline void parse_op_sum(tree_t *op) {
   parse_op_binary(op, KEYWRD_OF, msgs.err.syn.op.sum);
}

static inline void parse_op_diff(tree_t *op) {
   parse_op_binary(op, KEYWRD_BTW, msgs.err.syn.op.diff);
}

static inline void parse_op_prod(tree_t *op) {
   parse_op_binary(op, KEYWRD_OF, msgs.err.syn.op.prod);
}

static inline void parse_op_quot(tree_t *op) {
   parse_op_binary(op, KEYWRD_BTW, msgs.err.syn.op.quot);
}

static inline void parse_op_rem(tree_t *op) {
   /* the remainder of
         the quotient between <const> and <const> */

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

static inline void parse_op_sqrt(tree_t *op) {
   parse_op_unary(op, msgs.err.syn.op.sqrt);
}

static inline void parse_op_squr(tree_t *op) {
   parse_op_unary(op, msgs.err.syn.op.squr);
}

static inline void parse_op_cube(tree_t *op) {
   parse_op_unary(op, msgs.err.syn.op.cube);
}

static inline void parse_op_2x(tree_t *op) {
   parse_const(op);
}

static inline void parse_op_fact(tree_t *op) {
   parse_op_unary(op, msgs.err.syn.op.fact);
}

static bool is_name(void) {
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
   size_t orig_idx;

   /* We assume that all names are unique, i.e. there is
      no overlap like "the Romeo" and "the Romeo Rome" */

   dp = tree_child(pt, 1);
   dpsiz = tree_clen(dp);

   orig_idx = idx;
   reason = "incomplete name";

   for (i = 0; i < dpsiz; i++) {
      ch = tree_child(dp, i);
      chsiz = tree_clen(ch);

      for (k = 0; k < chsiz; k++) {
         ch_subnode = tree_chdat(ch, k);
         if (strcmp(ch_subnode->dat.s.run, tok->run)) {
            idx = orig_idx;
            tok = array_peek(toks, idx);  /* bracktrack */
            break;
         }
         gettok();
      }

      if (k == chsiz) {
         ungettok();
         charidx = i;
         return true;
      }
   }

   return false;
}

static bool is_name_lower(void) {
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

   if(is_name())
      return 1;
   tok->run[0] = tolower(tok->run[0]);
   return 0;
}

static int seek_line(void) {
   return is_name();
}

static void parse_line(void) {
   reason = msgs.err.syn.line.incomp;
   gettok();

   if (tok->run[0] != ':') {
      reason = "a colon expected after a character name to construct a line";
      synerr();
   }

   /* Note. charidx has been updated by is_name() in seek_line() */
   line = graft_tree_n(scene, 0, NODEKIND_LINE);
   (void) graft_tree_n(line, charidx, NODEKIND_CHAR);

   /* Handles the first statement */
   reason = msgs.err.syn.line.incomp;
   gettok();

   archive_tokstate();

   if (parse_line_router(stmts, stmts_len)) {
      reason = msgs.err.syn.line.nostmt;
      synerr();
   }

   /* Handles the rest */
   for (;;) {
      if (idx == len - 1)
         JUMP(NODEKIND__FINALE);

      /* if this token is the beginning of another line,
         then longjmp happens inside `seek_stmt_router` */
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
   const stmthandler_t *table = stmts;
   size_t tsiz = stmts_len;

   reason = msgs.err.syn.ifstmt.conseq_incomp;
   gettok();

   archive_tokstate();

   if (isupper(tok->run[0]))
      return 2; // fixme: 여기서 그냥 오류 처리하기
   else
   if (islower(tok->run[0]))
      tok->run[0] = toupper(tok->run[0]);

   /* 'if' statement can't have an 'if' statement as a consequent */
   if (1) {
      table = stmts + 1;
      tsiz = stmts_len - 1;
   }

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
   /* TYPE 1: You A(.|!)
      TYPE 2: You be as adj as (B|C|D)(.|!)
      TYPE 3: You be (B|C|D)(.|!) */

   tree_t *asgn;
   token_t *you;

   /* It's obvious that the current tok->run is either
      "you" or "thou" because of `seek_asgn()` */
   you = tok;

   reason = msgs.err.syn.asgn.incomp;
   gettok();

   if (is_be_conjs(tok->run)) {  /* type 2 or 3 */
      gettok();

      if (!strcmp(tok->run, "as"))
         asgn = parse_asgn_ii(you);  /* type 2 */
      else {
         ungettok();
         asgn = parse_asgn_iii(you);  /* type 3 */
      }
   }
   else {  /* type 1 */
      ungettok();
      asgn = parse_asgn_i(you);
   }

   parse_const(asgn);

   if (strchr(".!", tok->run[0]))
      return;
   reason = msgs.err.syn.asgn.invalid_end_symbol;
   synerr();
}

static bool is_be_conjs(const char *str) {
   static const char *conjugations[] = {
      "am", "are", "art", "is", "be", NULL
   };

   for (size_t i = 0; conjugations[i]; i++)
      if (!strcmp(str, conjugations[i]))
         return true;
   return false;
}

static tree_t *parse_asgn_i(token_t *you) {
   /* TYPE 1: You A(.|!) */
   return graft_tree_s(line, you->run, you->len, NODEKIND_ASGN1);
}

static tree_t *parse_asgn_ii(token_t *you) {
   /* TYPE 2: You be as adj as (B|C|D)(.|!) */
   reason = msgs.err.syn.asgn.incomp;
   gettok();

   if (!query_adj(tok->run)) {
      reason = msgs.err.syn.asgn.not_adj;
      synerr();
   }

   reason = msgs.err.syn.asgn.incomp;
   gettok();

   if (strcmp(tok->run, "as")) {
      reason = msgs.err.syn.asgn.not_as;
      synerr();
   }

   return graft_tree_s(line, you->run, you->len, NODEKIND_ASGN2);
}

static tree_t *parse_asgn_iii(token_t *you) {
   /* TYPE 3: You be (B|C|D)(.|!) */
   return graft_tree_s(line, you->run, you->len, NODEKIND_ASGN3);
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
   if (!strcmp(tok->run, KEYWRD_ACT_L))
      mark = NODEKIND_ACT;
   else
   if (!strcmp(tok->run, KEYWRD_SCENE_L))
      mark = NODEKIND_SCENE;
   else {
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
   if (vtype == cond_verbs_len)
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

   /* Note: [x] = current token
      type 0: [Am] I not better than yourself?
      type 2: [Art] thou not more cunning than the Ghost?
      type 3: [Is] a tree not as good as a shiny tree? */

   condition = graft_tree_n(line, 0, NODEKIND_COND);
   lefthand  = graft_tree_n(condition, 0, NODEKIND_LHS);

   reason = msgs.err.syn.cond.incomp;
   gettok();

   /* Am [I] not better than yourself?
      Art [thou] not more cunning than the Ghost?
      Is [a] tree not as good as a shiny tree? */

   switch (vtype) {
      case 0 : if (strcmp(tok->run, KEYWRD_I))
                  goto hell; else break;
      case 1 : if (strcmp(tok->run, KEYWRD_YOU_L))
                  goto hell; else break;
      case 2 : if (strcmp(tok->run, KEYWRD_THOU_L))
                  goto hell; else break;
      /* i.e. check if "Is (I, you, thou)" */
      case 3 : if (match_str(tok->run, cond_subjs, cond_subjs_len) < cond_subjs_len)
                  goto hell; else break;
      hell : /* FLAMING HOT */
         reason = msgs.err.syn.cond.not_conj;
         synerr();
   }

   if (vtype == 3) {
      /* need to ungettok before parse_const() */
      ungettok();
      /* [Is] a tree not as good as a shiny tree? */
   }

   switch (vtype) {
      case 0  : kind = NODEKIND_P1; break;
      case 1  : /* fall-through */
      case 2  : kind = NODEKIND_P2; break;
      case 3  : kind = NODEKIND_P3; break;
      /* control never reaches default */
      default : kind = NODEKIND__UNKNOWN;
   }
   p = graft_tree_n(lefthand, 0, kind);

   if (vtype == 3) {
      /* parse_const() has consumed "a tree" */
      parse_const(p);
      /* Is a tree [not] as good as a shiny tree? */
   } else {
      reason = msgs.err.syn.cond.incomp;
      gettok();
      /* Am I [not] better than yourself?
         Art thou [not] more cunning than the Ghost? */
   }

   if (!strcmp(tok->run, KEYWRD_NOT)) {
      (void) graft_tree_n(condition, 0, NODEKIND_NEGATE);
      reason = msgs.err.syn.cond.incomp;
      gettok();
      /* Am I not [better] than yourself?
         Art thou not [more] cunning than the Ghost?
         Is a tree not [as] good as a shiny tree? */
   }
   else
      (void) graft_tree_n(condition, 0, NODEKIND_AFFIRM);

   if (!strcmp(tok->run, KEYWRD_AS)) {
      parse_cond_eq(condition);
      /* Is a tree not as good [as] a shiny tree? */
   }
   else {
      parse_cond_ineq(condition);
      /* Am I not better [than] yourself?
         Art thou not more cunning [than] the Ghost? */
   }

   righthand = graft_tree_n(condition, 0, NODEKIND_RHS);
   /* The last noun is to be comsumed */
   parse_const(righthand);
   /* Am I not better than yourself[?]
      Art thou not more cunning than the Ghost[?]
      Is a tree not as good as a shiny tree[?] */
}

static void parse_cond_eq(tree_t *cond) {
   /* Is a tree not [as] good as a shiny tree? */
   gettok();
   /* Is a tree not as [good] as a shiny tree? */

   if (tok->kind == TOKKIND_PNT) {
      reason = msgs.err.syn.cond.badsyn;
      synerr();
   }

   (void) graft_tree_s(
      cond, tok->run, tok->len, NODEKIND_EQ);

   gettok();
   /* Is a tree not as good [as] a shiny tree? */

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

   /* Art thou not [more] cunning than the Ghost?
      Am I not [better] than yourself? */

   ret = match_str(tok->run, comps, comps_len);

   /* like "more beautiful" or "less interesting" */
   if (ret < comps_len) {
      reason = msgs.err.syn.cond.incomp;
      gettok();
      /* Art thou not more [cunning] than the Ghost? */

      if (tok->kind == TOKKIND_PNT) {
         reason = msgs.err.syn.cond.badsyn;
         synerr();
      }

      switch (ret) {
         case 0 : kind = NODEKIND_GT; break;
         case 1 : kind = NODEKIND_LT; break;
         default: ;  /* control never reaches default */
      }

      (void) graft_tree_s(cond, tok->run, tok->len, kind);
   }
   /* one word comparative like "better" or "worse" */
   else
      (void) graft_tree_s(
         cond, tok->run, tok->len, NODEKIND_INEQ);

   reason = msgs.err.syn.cond.incomp;
   gettok();
   /* Am I not better [than] yourself?
      Art thou not more cunning [than] the Ghost? */

   if (strcmp(tok->run, KEYWRD_THAN)) {
      reason = msgs.err.syn.cond.badsyn;
      synerr();
   }
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
   ungettok();

   parse_const(push);

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
   tok = array_peek(toks, idx);
}

static void gettok(void) {
   nexttok();
   etok = tok;
}

static void gettokn(int n) {
   if (idx + n >= len) synerr();
   idx += n;
   tok = array_peek(toks, idx);
   etok = tok;
}

static void ungettok(void) {
   tok = array_peek(toks, --idx);
   etok = tok;
}

static void ungettokn(int n) {
   idx -= n;
   tok = array_peek(toks, idx);
   etok = tok;
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

static void readtoks_until(char *scanset, tree_t *t) {
   size_t i = 0;

   for (;;) {
      nexttok();

      while (scanset[i] != '\0') {
         if (tok->run[0] == scanset[i]) {
            return;
         }
         i++;
      }

      graft_tree_s(t, tok->run, tok->len, NODEKIND_DATA);
      i = 0;
   }
}

static inline void archive_tokstate(void) {
   tidx = idx;
}

static inline void rewind_tokstate(void) {
   idx = tidx;
   tok = array_peek(toks, idx);
}

static void synerr(void) {
   int lnum, lpos;
   line_t *l;

   lnum = etok->lnum;
   lpos = etok->lpos;
   l = array_peek(ls, lnum - 1);

   safe_vprintf(
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
      buf = safe_malloc(len);  /* already has a room for \0 */
      memcpy(buf, run, len);
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
   // fixme: consider not to rely on tok
   // 매개변수로 lnum lpos 받기
   tree_t *sub = plant_tree(
      run, len, kind,
      tok->lnum, tok->lpos
   );

   return tree_graft(base, sub);
}

static tree_t *graft_tree_n(
   tree_t *base,
   int val,
   nodekind_t kind
) {
   node_t node;
   tree_t *sub;

   node.datkind = DATKIND_INT;
   node.dat.n = val;
   node.kind = kind;
   node.lnum = tok->lnum; // fixme: 매개변수로 lnum lpos 받기
   node.lpos = tok->lpos;
   sub = tree_plant(&node, sizeof node);

   return tree_graft(base, sub);
}

extern void setndn(node_t *n, int v) {
   n->datkind = DATKIND_INT;
   n->dat.n = v;
}

extern void setnds(node_t *n, char *s, int l) {
   n->datkind = DATKIND_STR;
   n->dat.s.run = s;
   n->dat.s.len = l;
}
