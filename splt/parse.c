#include "parse.h"
#include "parse.type.h"  // contains typedef & prototypes

/******************************
 * IMPORTANT GLOBAL VARIABLES *
 ******************************/
/* Line Access */
static line_t *ls;   // array of line_t
static int lls;      // length of ls

static int p;        // line number
static int q;        // position in line
static line_t *l;    // l = &ls[p]

static int tp;       // temp. var. for p
static int tq;       // temp. var. for q
static line_t *tl;   // temp. var. for l

/* Line Contents Copy */
static char ch;      // to store a char
static char *buf;    // to store a string
static int idx;      // position in buf
static int max;      // size of buf

/* Miscellaneous */
static const char *sfname;   // name of source file

extern void parse(
   optflg_t *of, optval_t *ov,
   line_t *arr, int len,
   tree_t *pt
) {
   // Initialize global variables
   ls = arr;
   lls = len;
   p = q = 0;
   l = &ls[p];
   max = 128;
   buf = smalloc(max);
   sfname = ov->src;

   // Construct the parse tree
   tree_t *act, *scene;
   int ret;

   parse_title(pt);
   parse_dp(pt);
   for (;;) {
      act = parse_act(pt);
      for (;;) {
         scene = parse_scene(act);
         for (;;) {
            ret = parse_stmt(scene);
            switch (ret) {
               case RET_EOT: goto EOT;
               case RET_EOS: goto EOS;
               case RET_EOA: goto EOA;
               case RET_EOE: goto EOE;
            }
            EOT:;
         }
         EOS:;
      }
      EOA:;
   }
   EOE:;

   // Cleanup
   free(buf);
}

static int seek_actlike(const char *type) {
   /*
    * <type> := Act | Scene
    * target := <type>
    */
   store_nchar(strlen(type));

   return !strcmp(buf, type) ? 1 : 0;
}

static int seek_act(void) {
   return seek_actlike("Act");
}

static int seek_scene(void) {
   return seek_actlike("Scene");
}

static int seek_enterlike(const char *type) {
   /*
    * <ws> := ' ' | \(a|b|t|n|v|f|r)
    * <type> := Enter | Exit | Exeunt
    * target := [<ws*><type>
    */
   store_nchar(1);

   if (ch != '[')
      return 0;

   leave_space();
   store_nchar(strlen(type));

   return !strcmp(buf, type) ? 1 : 0;
}

static int seek_enter(void) {
   return seek_enterlike("Enter");
}

static int seek_exit(void) {
   return seek_enterlike("Exit");
}

static int seek_exeunt(void) {
   return seek_enterlike("Exeunt");
}

static int seek_line(void) {
   /*
    * target := <name><ws*>:
    */
   store_until(':');
   leave_space();
   return 1;
}

static int seek_asgn_i(void) {
   /*
    * target := (You | Thou)<ws+>
    */
   store_nchar(4);

   if (strncmp(buf, "You", 3) && strcmp(buf, "Thou"))
      return 0;

   leave_space();

   return 1;
}

static int seek_asgn_ii(void) {
   /*
    * target := ("You are" | "Thou art")<ws+>as<ws+>
    */
   store_nchar(8);

   if (strncmp(buf, "You are", 7) && strcmp(buf, "Thou art"))
      return 0;

   store_nchar(1);

   if (!isspace(ch))
      return 0;

   leave_space();
   store_nchar(2);

   if (strcmp(buf, "as"))
      return 0;

   store_nchar(1);

   if (!isspace(ch))
      return 0;

   leave_space();

   return 1;
}

static int seek_token(const char *type) {
   store_nchar(strlen(type));

   if (strcmp(buf, type))
      return 0;

   store_nchar(1);

   if (!isspace(ch))
      return 0;

   leave_space();

   return 1;
}

static int seek_iolike_i(const char *type) {
   /*
    * <type> := Speak | Listen
    * target := <type><ws+>
    */
   return seek_token(type);
}

static int seek_iolike_ii(char sentinel) {
   /*
    * <type> := h|m
    * target := Open<ws+>(your | YOUR)<ws+><type>
    */
   if (!seek_iolike_i("Open"))
      return 0;

   store_nchar(4);

   if (strcmp(buf, "your") && strcmp(buf, "YOUR")) {
      reason = msgs.err.out_i_wrong_syntax;
      synerr(tell);
   }

   store_nchar(1);

   if (!isspace(ch)) {
      reason = msgs.err.out_i_no_space;
      synerr(tell);
   }

   leave_space();
   store_nchar(1);

   return ch == sentinel ? 1 : 0;
}

static int seek_out_i(void) {
   return seek_iolike_ii('h');
}

static int seek_out_ii(void) {
   return seek_iolike_i("Speak");
}

static int seek_in_i(void) {
   return seek_iolike_i("Listen");
}

static int seek_in_ii(void) {
   return seek_iolike_ii('m');
}

static int seek_goto(void) {
   /*
    * target := (We | Let)<ws+>
    */
   store_nchar(3);

   if (strncmp(buf, "We", 2) && strcmp(buf, "Let"))
      return 0;

   leave_space();

   return 1;
}

static int seek_anteclike(const char *type) {
   /*
    * target := (Am | Are | Art | Is)<ws+>
    */
   return seek_token(type);
}

static int seek_antec_i(void) {
   return seek_anteclike("Am");
}

static int seek_antec_ii(void) {
   return seek_anteclike("Are");
}

static int seek_antec_iii(void) {
   return seek_anteclike("Art");
}

static int seek_antec_iv(void) {
   return seek_anteclike("Is");
}

static int seek_conselike(const char *type) {
   /*
    * target := If<ws+>(so | not)<ws*>
    */
   store_nchar(2);

   if (strcmp(buf, "If"))
      return 0;

   store_nchar(strlen(type));

   if (strcmp(buf, type))
      return 0;

   leave_space();

   return 1;
}

static int seek_conse_i(void) {
   return seek_conselike("so");
}

static int seek_conse_ii(void) {
   return seek_conselike("not");
}

static int seek_push(void) {
   /*
    * target := Remember<ws+>
    */
   return seek_token("Remember");
}

static int seek_pop(void) {
   /*
    * target := Recall<ws+>
    */
   return seek_token("Recall");
}

static void parse_title(tree_t *pt) {
   /*
    * target := <ws*><letter*>.<ws*>
    */
   reason = msgs.err.notitle;
   leave_space();

   reason = msgs.err.ontitle;
   store_until('.');
   leave_nchar(1);

   reason = msgs.err.eof;
   leave_space();

   // Construct the parse tree
   if (isspace(buf[idx - 1]))
      trim(buf);  /* removes trailing spaces */

   (void) tree_sgraft(pt, buf, "title");
}

static void parse_dp(tree_t *pt) {
   /*
    * character := <name><ws*>,<description>.
    * target := <character+><ws*>
    */
   tree_t *dp;

   dp = tree_plant(NULL, 0);
   strcpy(dp->tag, "dp");

   // Construct the dramatis personae
   for (;;) {
      reason = msgs.err.onname;
      store_until(',');

      reason = msgs.err.onchardesc;
      leave_until('.');
      leave_nchar(1);

      reason = msgs.err.eof;
      leave_space();

      // Construct a tree for the name
      normalize(buf);
      (void) tree_sgraft(dp, buf, "chardecl");

      // Checks if this is the end of dramatis personae
      save_state();
      if (seek_act())
         break;
      load_state();
   }

   // Construct the parse tree
   tree_addchild(pt, dp);
}

static tree_t *parse_act(tree_t *pt) {
   /*
    * <roman-num> := I|V|X|L|C|D|M
    * target := <ws+><roman-num><ws*>:<letter*>.<ws*>
    */
   reason = msgs.err.act_unfinished;
   store_nchar(1);

   if (!isspace(buf[0])) {
      reason = msgs.err.act_no_space;
      synerr(tell);
   }

   leave_space();
   store_roman();
   leave_space();
   store_nchar(1);

   if (buf[0] != ':') {
      reason = msgs.err.act_no_colon;
      synerr(tell);
   }

   reason = msgs.err.act_desc_unfinished;
   leave_until('.');
   store_nchar(1);

   if (buf[0] != '.') {
      reason = msgs.err.act_no_period;
      synerr(tell);
   }

   reason = msgs.err.eof;
   leave_space();

   // Construct the parse tree
   // buf has gotten the act number in seek_act
   return tree_graft(pt, buf, idx + 1, "act");
}

static tree_t *parse_scene(tree_t *act) {
   /*
    * target := <letter*>.<ws*>
    */
   reason = msgs.err.onscenedesc;
   leave_until('.');

   reason = msgs.err.eof;
   leave_space();

   // Construct the parse tree
   // buf has gotten the scene number in seek_scene
   return tree_graft(act, buf, idx + 1, "scene");
}

static int parse_stmt(tree_t *scene) {
   typedef struct finalehandler {
      seeker_pt seek;
      int retval;
   } finalehandler_t;

   static const stmthandler_t shs[] = {
      { seek_enter , parse_enter  },
      { seek_exit  , parse_exit   },
      { seek_exeunt, parse_exeunt },
      { seek_line  , parse_line   }
   };
   const int shslen = sizeof shs / sizeof shs[0];
   const stmthandler_t *sh;

   static const finalehandler_t fhs[] = {
      { seek_scene, RET_EOS },
      { seek_act  , RET_EOA }
   };
   const int fhslen = sizeof fhs / sizeof fhs[0];
   const finalehandler_t *fh;

   if (p == lls)
      return RET_EOE;

   reason = msgs.err.eof;

   for (int i = 0; i < shslen; i++) {
      save_state();
      sh = shs + i;
      if ((*sh->seek)())
         return sh->parse(scene);
      load_state();
   }

   for (int i = 0; i < fhslen; i++) {
      save_state();
      fh = fhs + i;
      if ((*fh->seek)())
         return fh->retval;
      load_state();
   }

   reason = msgs.err.inctok;
   synerr(tell);
   return -1;  /* control never reaches here */
}

static int parse_enter(tree_t *scene) {
   /*
    * <lastname> := and<ws+><name><ws*>
    * target := (<name><ws+><lastname>]<ws*>)
    *    | (<name><ws*>,<ws+>)+<lastname>)
    */
   reason = msgs.err.enter_on_stmt;
   store_nchar(1);

   if (ch == ']') {
      reason = msgs.err.noenterchar;
      synerr(tell);
   }
   if (!isspace(ch)) {
      reason = msgs.err.nowsafterenter;
      synerr(tell);
   }

   /*
    * Store the names specified in Enter
    * Example:
    *    input = [Enter the Ghost, Romeo, and Juliet]
    *    output = "the Ghost", "Romeo", "Juliet"
    */
   reason = msgs.err.enter_on_stmt;
   store_until(']');

   if (!strlen(buf)) {
      reason = msgs.err.noenterchar;
      synerr(tell);
   }

   reason = msgs.err.eof;
   leave_space();

   // Construct the parse tree
   reason = msgs.err.enter_no_connective;
   (void) interpret_namelist(scene, "enter", "name");

   return RET_EOT;
}

static int parse_exit(tree_t *scene) {
   /*
    * target := <letter*><ws*>]<ws*>
    */
   reason = msgs.err.exit_on_stmt;
   store_nchar(1);

   if (ch == ']') {
      reason = msgs.err.exit_no_char;
      synerr(tell);
   }
   if (!isspace(ch)) {
      reason = msgs.err.exit_no_space_after;
      synerr(tell);
   }

   reason = msgs.err.exit_on_stmt;
   store_until(']');

   if (!strlen(buf)) {
      reason = msgs.err.exit_no_char;
      synerr(tell);
   }

   reason = msgs.err.eof;
   leave_space();

   // Construct the parse tree
   tree_t *exit_stmt, *exit_char;

   exit_stmt = tree_plant(NULL, 0);
   strcpy(exit_stmt->tag, "exit");

   normalize(buf);
   exit_char = tree_plant(buf, strlen(buf) + 1);
   strcpy(exit_char->tag, "exit char");

   tree_addchild(exit_stmt, exit_char);
   tree_addchild(scene, exit_stmt);

   return RET_EOT;
}

static int parse_exeunt(tree_t *scene) {
   /*
    * target := ] | <letter*><ws*>]<ws*>
    */
   reason = msgs.err.exeunt_on_stmt;
   store_nchar(1);

   if (ch == ']')
      goto exeunt_no_char_specified;

   if (!isspace(ch)) {
      reason = msgs.err.exeunt_no_space_after;
      synerr(tell);
   }

   // As in parse_enter, store the names
   reason = msgs.err.exeunt_on_stmt;
   store_until(']');

   if (!strlen(buf)) {
      exeunt_no_char_specified:;
      (void) tree_graft(scene, NULL, 0, "exeunt");
      return RET_EOT;
   }

   reason = msgs.err.eof;
   leave_space();

   // Construct the parse tree
   tree_t *exeunt_stmt;

   reason = msgs.err.exeunt_no_connective;
   exeunt_stmt = interpret_namelist(scene, "exeunt", "name");

   if (exeunt_stmt->clen < 2) {
      reason = msgs.err.exeunt_only_one_name;
      synerr(tell);
   }

   return RET_EOT;
}

static int parse_line(tree_t *scene) {
   static const stmthandler_t shs[] = {
      { seek_asgn_i   , parse_asgn_i    },
      { seek_asgn_ii  , parse_asgn_ii   },
      { seek_out_i    , parse_out_i     },
      { seek_out_ii   , parse_out_ii    },
      { seek_in_i     , parse_in_i      },
      { seek_in_ii    , parse_in_ii     },
      { seek_goto     , parse_goto      },
      { seek_antec_i  , parse_antec_i   },
      { seek_antec_ii , parse_antec_ii  },
      { seek_antec_iii, parse_antec_iii },
      { seek_antec_iv , parse_antec_iv  },
      { seek_conse_i  , parse_conse_i   },
      { seek_conse_ii , parse_conse_ii  },
      { seek_push     , parse_push      },
      { seek_pop      , parse_pop       }
   };
   const int len = sizeof shs / sizeof shs[0];

   const stmthandler_t *sh;
   tree_t *line;
   int i;

   normalize(buf);
   line = tree_sgraft(scene, buf, "line");

   for (;;) {
      for (i = 0; i < len; i++) {
         save_state();
         sh = shs + i;
         if (sh->seek())
            return sh->parse(line);
         load_state();
      }

      reason = msgs.err.inctok;
      synerr(tell);
   }
}

static int parse_asgn_i(tree_t *line) {
   /*
    * <adj> := <letter*><ws+>
    * <noun> := <letter*><ws*>
    * <reflexive-pronoun> := myself | yourself | thyself
    * <pronoun> := me | you | thee
    * <const> := (<adj*><noun>) | <reflexive-pronoun>
    *     | <pronoun> | <name>
    * target := <const>!
    */
   reason = msgs.err.asgn_i_unfinished;
   store_until('!');

   // Construct the parse tree
   interpret_constant(line);

   return RET_EOT;
}

static int parse_asgn_ii(tree_t *line) {
   /*
    * target := <adj><ws+>as<ws+><const>.
    */
   reason = msgs.err.asgn_ii_unfinished;
   store_until_space();

   // TODO: validate the adjective
   if (0) {
      reason = msgs.err.asgn_ii_invalid_adj;
      synerr(tell);
   }

   store_nchar(1);

   if (!isspace(ch)) {
      reason = msgs.err.asgn_ii_no_space;
      synerr(tell);
   }

   leave_space();
   store_nchar(2);

   if (strcmp(buf, "as")) {
      reason = msgs.err.asgn_ii_no_as;
      synerr(tell);
   }

   store_nchar(1);

   if (!isspace(ch)) {
      reason = msgs.err.asgn_ii_no_space;
      synerr(tell);
   }

   leave_space();

   reason = msgs.err.asgn_ii_unfinished;
   store_until('.');

   // Construct the parse tree
   interpret_constant(line);

   return RET_EOT;
}

static int parse_out_i(tree_t *line) {
   /*
    * target := eart<ws*>(.|!)<ws*>
    */
   reason = msgs.err.out_i_unfinished;
   store_nchar(4);

   if (strcmp(buf, "eart")) {
      reason = msgs.err.out_i_wrong_syntax;
      synerr(tell);
   }

   leave_space();
   store_nchar(1);

   if (ch != '.' && ch != '!') {
      reason = msgs.err.out_i_wrong_syntax;
      synerr(tell);
   }

   leave_space();

   // Construct the parse tree
   tree_graft(line, NULL, 0, "print-int");

   return RET_EOT;
}

static int parse_out_ii(tree_t *line) {
   /*
    * target := (your | YOUR)<ws+>mind<ws*>(.|!)<ws*>
    */
   reason = msgs.err.out_ii_unfinished;
   store_nchar(4);

   if (strcmp(buf, "your") && strcmp(buf, "YOUR")) {
      reason = msgs.err.out_ii_wrong_syntax;
      synerr(tell);
   }

   store_nchar(1);

   if (!isspace(ch)) {
      reason = msgs.err.out_ii_no_space;
      synerr(tell);
   }

   leave_space();
   store_nchar(4);

   if (strcmp(buf, "mind")) {
      reason = msgs.err.out_ii_wrong_syntax;
      synerr(tell);
   }

   leave_space();
   store_nchar(1);

   if (ch != '.' && ch != '!') {
      reason = msgs.err.out_ii_wrong_syntax;
      synerr(tell);
   }

   leave_space();

   // Construct the parse tree
   tree_graft(line, NULL, 0, "print-char");

   return RET_EOT;
}

static int parse_in_i(tree_t *line) {
   /*
    * target := to<ws+>(your | YOUR)<ws+>heart<ws*>(.|!)<ws*>
    */
   reason = msgs.err.in_i_unfinished;
   store_nchar(2);

   if (strcmp(buf, "to")) {
      reason = msgs.err.in_i_wrong_syntax;
      synerr(tell);
   }

   reason = msgs.err.in_i_unfinished;
   store_nchar(4);

   if (strcmp(buf, "your") && strcmp(buf, "YOUR")) {
      reason = msgs.err.in_i_wrong_syntax;
      synerr(tell);
   }

   store_nchar(1);

   if (!isspace(ch)) {
      reason = msgs.err.in_i_no_space;
      synerr(tell);
   }

   leave_space();
   store_nchar(5);

   if (strcmp(buf, "heart")) {
      reason = msgs.err.in_i_wrong_syntax;
      synerr(tell);
   }

   leave_space();
   store_nchar(1);

   if (ch != '.' && ch != '!') {
      reason = msgs.err.in_i_wrong_syntax;
      synerr(tell);
   }

   leave_space();

   // Construct the parse tree
   tree_graft(line, NULL, 0, "scan-int");

   return RET_EOT;
}

static int parse_in_ii(tree_t *line) {
   /*
    * target := ind<ws*>(.|!)<ws*>
    */
   reason = msgs.err.in_ii_unfinished;
   store_nchar(3);

   if (strcmp(buf, "ind")) {
      reason = msgs.err.in_ii_wrong_syntax;
      synerr(tell);
   }

   leave_space();
   store_nchar(1);

   if (ch != '.' && ch != '!') {
      reason = msgs.err.in_ii_wrong_syntax;
      synerr(tell);
   }

   leave_space();

   // Construct the parse tree
   tree_graft(line, NULL, 0, "scan-char");

   return RET_EOT;
}

static int parse_goto(tree_t *line) {
   /*
    * <initiator> := us | shall | must
    * <body> := "return to" | "proceed to"
    * <scene> := scene<ws+><roman-num>
    * target := <initiator><ws+><body><ws+><scene><ws*>.<ws*>
    */
   reason = msgs.err.goto_unfinished;
   store_nchar(5);

   if (strncmp(buf, "us", 2)
      && strncmp(buf, "must", 4)
      && strcmp(buf, "shall"))
   {
      reason = msgs.err.goto_synerr;
      synerr(tell);
   }

   store_nchar(1);

   if (!isspace(ch)) {
      reason = msgs.err.goto_no_space;
      synerr(tell);
   }

   leave_space();
   store_nchar(10);

   if (strncmp(buf, "return to", 9)
      && strcmp(buf, "proceed to"))
   {
      reason = msgs.err.goto_synerr;
      synerr(tell);
   }

   store_nchar(1);

   if (!isspace(ch)) {
      reason = msgs.err.goto_no_space;
      synerr(tell);
   }

   leave_space();
   store_nchar(5);

   if (strcmp(buf, "scene")) {
      reason = msgs.err.goto_synerr;
      synerr(tell);
   }

   store_nchar(1);

   if (!isspace(ch)) {
      reason = msgs.err.goto_no_space;
      synerr(tell);
   }

   leave_space();
   store_roman();
   leave_until('.');
   leave_space();

   // Construct the parse tree
   tree_sgraft(line, buf, "goto");

   return RET_EOT;
}

static int parse_antec_common(tree_t *line) {
   /*
    * target := <ws+>[not]<ws+><comparative><ws+>
    *    than<ws+><const><ws*>?<ws*>
    */
   reason = msgs.err.antec_unfinished;
   store_nchar(1);

   if (!isspace(ch)) {
      reason = msgs.err.antec_no_space;
      synerr(tell);
   }

   leave_space();
   save_state();
   store_nchar(4);

   if (!strncmp(buf, "not", 3) && isspace(ch))
      tree_graft(line, NULL, 0, "not");
   else
      load_state();

   store_until_space();
   tree_sgraft(line, buf, "comparative");
   store_nchar(1);

   if (!isspace(ch)) {
      reason = msgs.err.antec_no_space;
      synerr(tell);
   }

   leave_space();
   store_nchar(4);

   if (strcmp(buf, "than")) {
      reason = msgs.err.antec_bad_syntax;
      synerr(tell);
   }

   store_nchar(1);

   if (!isspace(ch)) {
      reason = msgs.err.antec_no_space;
      synerr(tell);
   }

   leave_space();
   store_until('?');
   leave_space();

   // Construct the parse tree
   interpret_constant(line);

   return 1;
}

static void parse_anteclike(const char *type) {
   reason = msgs.err.antec_unfinished;
   store_nchar(strlen(type));

   if (strcpy(buf, type)) {
      reason = msgs.err.antec_not_conjugated;
      synerr(tell);
   }
}

static int parse_antec_i(tree_t *line) {
   parse_anteclike("I");
   tree_sgraft(line, "first-person", "left-side");
   return parse_antec_common(line);
}

static int parse_antec_ii(tree_t *line) {
   parse_anteclike("you");
   tree_sgraft(line, "second-person", "left-side");
   return parse_antec_common(line);
}

static int parse_antec_iii(tree_t *line) {
   parse_anteclike("thou");
   tree_sgraft(line, "second-person", "left-side");
   return parse_antec_common(line);
}

static int parse_antec_iv(tree_t *line) {
   /*
    * target := <const>
    */
   tree_t *constant;

   constant = tree_graft(line, NULL, 0, "left-side");
   interpret_constant(constant);
   return parse_antec_common(line);
}

static int parse_conse_common(tree_t *line) {
   /*
    * target := ,<ws*><line>
    */
   tree_t *consequent;

   reason = msgs.err.conse_unfinished;
   store_nchar(1);

   if (ch != ',') {
      reason = msgs.err.conse_bad_syntax;
      synerr(tell);
   }

   leave_space();
   consequent = tree_graft(line, NULL, 0, "consequent");
   return parse_line(consequent);
}

static int parse_conse_i(tree_t *line) {
   tree_graft(line, NULL, 0, "affirm");
   return parse_conse_common(line);
}

static int parse_conse_ii(tree_t *line) {
   tree_graft(line, NULL, 0, "deny");
   return parse_conse_common(line);
}

static int parse_push(tree_t *line) {
   /*
    * target := (<pronoun> | <reflexive-pronoun>)<ws*>.<ws*>
    */
   reason = msgs.err.push_unfinished;
   store_until('.');
   leave_space();

   // Construct the parse tree
   normalize(buf);
   trim(buf);
   tree_sgraft(line, buf, "push");

   return 1;
}

static int parse_pop(tree_t *line) {
   /*
    * target := <ws*><letter*>.
    */
   reason = msgs.err.pop_unfinished;
   leave_until('.');

   // Construct the parse tree
   tree_graft(line, NULL, 0, "pop");

   return 1;
}

static inline void save_state(void) {
   tp = p, tq = q, tl = l;
}

static inline void load_state(void) {
   p = tp, q = tq, l = tl;
}

static void leave_nchar(int n) {
   iterate_lines(process_leave, check_cntlessthan, n);
}

static void leave_space(void) {
   iterate_lines(process_leave, check_space);
}

static void leave_until(char sentinel) {
   iterate_lines(process_leave, check_chnotequalto, sentinel);
}

static void store_nchar(int n) {
   store_template(check_cntlessthan, n);
}

static void store_until(char sentinel) {
   store_template(check_chnotequalto, sentinel);
}

static void store_until_space(void) {
   store_template(check_notspace);
}

static void store_roman(void) {
   store_template(check_roman);
}

static void store_template(checker_t checker, ...) {
   va_list apst;  /* st = store template */

   idx = 0;
   va_start(apst, checker);
   iterate_lines(process_store, checker, &apst);
   va_end(apst);
}

static inline void iterate_lines(processor_t process, ...) {
   va_list apil;  /* il = iterate lines */

   while (p < lls) {
      while (q < l->len) {
         va_start(apil, process);
         if (process(&apil)) {
            va_end(apil);
            continue;
         }
         goto end;
      }
      q = 0;
      l = ls + ++p;  /* &ls[++p] */
   }
   synerr(tell_eoe);  /* p == lls */

   end: return;
}

static int process_leave(va_list *apil) {
   checker_pt check
      = va_arg(*apil, checker_pt);

   ch = l->run[q];
   if (check(apil)) {
      q++;
      return 1;
   }
   else
      return 0;
}

static int process_store(va_list *apil) {
   checker_pt check;
   va_list *apst, apst_temp;
   bool flag;

   check = va_arg(*apil, checker_pt);
   apst = va_arg(*apil, va_list *);
   va_copy(apst_temp, *apst);

   ch = l->run[q];
   if (check(&apst_temp)) {
      buf[idx++] = ch;
      if (idx == max) {
         max *= 2;
         buf = srealloc(buf, max);
      }
      q++;
      flag = true;
   }
   else {
      buf[idx] = '\0';
      flag = false;
   }

   va_end(apst_temp);
   return flag ? 1 : 0;
}

static int check_cntlessthan(va_list *ap) {
   static int count = 0;
   int n;

   n = va_arg(*ap, int);
   if (count < n) {
      count++;
      return 1;
   }
   else {
      count = 0;
      return 0;
   }
}

static int check_idxlessthan(va_list *ap) {
   int n;

   n = va_arg(*ap, int);
   if (idx < n) return 1;
   else return 0;
}

static int check_chnotequalto(va_list *ap) {
   char sentinel;

   sentinel = va_arg(*ap, int);  /* since promoted */
   if (ch != sentinel) return 1;
   else return 0;
}

static int check_space(va_list *_) {
   (void) _;
   if (isspace(ch)) return 1;
   else return 0;
}

static int check_notspace(va_list *_) {
   (void) _;
   if (!isspace(ch)) return 1;
   else return 0;
}

static int check_roman(va_list *_) {
   (void) _;
   if (match(ch, roman_numerals)) return 1;
   else return 0;
}

static void synerr(teller_pt tell) {
   ffmtwrt(stderr, Cbred "<syntax error> " Creset);
   (*tell)();
   exit(EXIT_FAILURE);
}

static void tell(void) {
   ffmtwrt(stderr,
      "%s\n"
      "[%s:%d:%d] " Cbwhite "note:" Creset " problematic since here\n"
      "%4d| %.*s" Cbblue "%s" Creset,
      reason,
      sfname, p, q,
      p, q, l->run, &l->run[q]
   );
}

static void tell_eoe(void) {
   l = ls + --p;  /* p == lls */
   ffmtwrt(stderr,
      "%s\n"
      "[%s:%d:%d] " Cbwhite "note:" Creset " reached end of source file\n"
      "%4d| %s" Cbblack "EOF" Creset "\n"
      "[%s:%d:%d] " Cbwhite "note:" Creset " problematic since here\n"
      "%4d| %.*s" Cbblue "%s" Creset,
      reason,
      sfname, l->num, l->len,
      l->num, l->run,
      sfname, tp, tq,
      tp, tq, tl->run, &tl->run[tq]
   );
}

static tree_t *interpret_namelist(
   tree_t *scene,
   char *stmt_tag,
   char *name_tag
) {
   tree_t *stmt;
   char **names, *pos, *last;
   int i, len;

   normalize(buf);
   names = split(buf, ",", &len);

   // regards the last element must contain " and "
   last = names[len - 1];
   pos = strstr(last, connective);
   if (!pos)
      // a correct reason must have been set
      synerr(tell);

   len++;
   names = srealloc(names, len * sizeof *names);

   for (i = 0; i < len - 1; i++)
      trim(names[i]);
   *pos = '\0';
   names[i] = last;
   names[i + 1] = pos + strlen(connective);

   stmt = tree_plant(NULL, 0);
   strcpy(stmt->tag, stmt_tag);

   for (i = 0; i < len; i++)
      (void) tree_sgraft(stmt, names[i], name_tag);
   tree_addchild(scene, stmt);

   for (i = 0; i < len; i++)
      free(names[i]);
   free(names);

   return stmt;
}

static void interpret_constant(tree_t *line) {
   int i, len;
   char **toks, *tok;

   normalize(buf);
   toks = split(buf, " ", &len);

   for (i = 0; i < len; i++)
      trim(toks[i]);

   for (i = 0; ((tok = toks[i]), i < len - 1); i++)
      (void) tree_sgraft(line, tok, "adj");
   (void) tree_sgraft(line, tok, "noun");

   for (i = 0; i < len; i++)
      free(toks[i]);
   free(toks);
}

static void traverse(tree_t *base) {
   static int level = 0;

   for (int i = 0; i < level; i++)
      fputs("  ", stdout);
   printf("[%s] = [%s]\n", base->tag,
      base->dsiz ? (char *) base->data : "(empty)");

   if (base->clen == 0) {
      level--;
      return;  /* end of recursion */
   }

   level++;
   for (int i = 0; i < base->clen; i++)
      traverse(base->child[i]);
}
