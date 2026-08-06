#include "parse.h"
#include "parse.internals.h"

extern tree_t *parse(optflg_t *of, optval_t *ov, array_t *tokens) {
   (void) of, (void) ov;
   // Initialize global variables
   toks = tokens;
   tok = array_peek(toks, 0);
   len = array_size(toks);
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
   array_foreach(toks, &cleanup_tokstream);
   array_destroy(toks);

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

   // fixme: enter can take one or more characters
   // 꼭 A,B, and C 이럴필요 X 공백은 어디든 있을수있음
   // 그냥 A B C 만 파싱하면 됨
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

// fixme: 명사가 인칭대명사인 경우 형용사나 관사가 있으면 오류처리
// 근데 이걸 parse_const에서 하기보다는 type-check에서 하는게 맞을듯?
// fixme: 추가로, typecheck시 소유격이나 관사가 등장한경우 오류 (parse_const시 혹시 맨앞에 있었던경우 이미 걸려졌으니까 없어야 함)
// query한 이후 사전에 없는경우 검사하는게 효율적일듯
// <= 이건 이미 이 함수 아랫단에서 검사되고 있는듯?
static void parse_const(tree_t *stmt) {
   static const char *decos[] = {
   /* Possessives */
      /* 1st */ KEYWRD_MY, KEYWRD_MINE,
      /* 2nd */ KEYWRD_YOUR, KEYWRD_THY, KEYWRD_THINE,
      /* 3rd */ KEYWRD_HIS, KEYWRD_HER, KEYWRD_ITS, KEYWRD_THEIR,
   /* Articles */
      KEYWRD_A, KEYWRD_AN, KEYWRD_THE
   };
   static const int decos_len = ARRLEN(decos);

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
         Be (B|C|D) <not> (comp|(<more|less> adj)) than (B|C|D)?

      3. As Operands Of Operators
         the sum of (B|C|D) and (B|C|D)

      4. Remember Statements
         Remember (B|C|D)(.|!)

      This remind shows us that TYPE A is only used in the
      "YOU A" assignment statement. Also, the combination of
      B, C, and D is widely used.

      With that in mind, now let's begin parsing. */

   tree_t *cnst;
   bool opcond, cond1, cond2, cond3, cond4, cond5, cond6;
   int ret;
   nodekind_t kind;

   /* Makes a tree that represents a constant node */
   cnst = graft_tree_n(stmt, 0, NODEKIND_CONST);

   reason = msgs.err.syn.cnst.incomp;
   gettok();

   /* First of all, we check whether this token is
      a pronoun, a reflexive, a name, or a nil */

   /* If not, this is either TYPE A or TYPE B. Meanwhile,
      TYPE B = (art|pos) + TYPE A. Let's exploit this structure */

   // if there is, ignore art|pos!

   // 형용사, 명사 체크를 여기서 해버리고 (명사구도 여기서해버리기)
   // context check에서는 conjugation이 제대로 됐는지같은걸 검사하도록 하자 (이건 warning을 띄워야 할듯)
   // context check에서는 미선언이름사용, 이름중복선언, 미사용이름체크, scene/act 중복선언 같은것을 체크
   // typecheck의 필요성이 없는거같은데 삭제 고려

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
   if (ret < decos_len)
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
      /* escape conditions */
      // fixme: ".!?" const static 변수로 빼기 (종결기호들)
      cond1 = match(tok->run[0], ".!?");
      if (cond1) goto noun;
      // fixme: STREQL 매크로 정의해서 쓰기 (!strcmp 대체 프로젝트 전체적으로)
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

      /* is it a decorator? */
      if (match_str(tok->run, decos, decos_len) < decos_len) {
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

// fixme: dp를 파싱한 다음에 바로 이름을 합치기
// 현재 typecheck.c에서 coalesce_name 하니까 isname에서 일일히 트리를 순회해야해서 불편함
// ㄴㄴ 토큰스트림 단계에서는 이렇게 하는게 맞을듯 여러 이름이 토큰에 나뉘어져 있으니까

// fixme: title 도 파싱한 다음에 바로 coalesce_title 해버리기 (typecheck.c에서 옮기기)
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
   // Make nodes (charidx has been updated by isname() in seek_line)
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
   /* ap: adjective phrase, e.g. "lying stupid fatherless"
      np: noun phrase, e.g. "summer's day", "stone wall"
      art: article, e.g. "a", "an", "the"
      pos: possessive, e.g. "my", "your"
      ref: reflexive, e.g. "myself", "yourself"
      pronoun: e.g. "me", "you", "I"
      name: e.g. "Romeo", "Juliet"

      You <ap> (noun|np)(.|!)
      You be as adj as (art|pos) <ap> (noun|np)(.|!)
      You be as adj as (pronoun|ref|name)(.|!)
      You be (nothing|zero)(.|!)

      Note: a noun phrase is treated as a single noun! */

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
}

static void ungettokn(int n) {
   idx -= n;
   tok = array_peek(toks, idx);
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
