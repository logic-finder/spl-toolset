#include "parse.h"
#include "parse.type.h"

/******************************
 * IMPORTANT GLOBAL VARIABLES *
 ******************************/
static tree_t *pt;
static list_t *toks;
static node_t *tok, *etok, *ttok;
static tree_t *act, *scene, *line;
static int idx, len, tidx;

// static const stmthandler_t enterlikes[] = {
//    { seek_enter , parse_enter  },
//    { seek_exit  , parse_exit   },
//    { seek_exeunt, parse_exeunt }
// };
// static const finalehandler_t finales[] = {
//    { seek_scene, RET_EOS },
//    { seek_act  , RET_EOA }
// };
// static const int hlen = ARRLEN(enterlikes);
// static const int flen = ARRLEN(finales);
// static const stmthandler_t *enterlike;
// static const finalehandler_t *finale;

static jmp_buf parse_env;

extern tree_t *parse(
   optflg_t *of,
   optval_t *ov,
   list_t *tokens
) {
   // Initialize global variables
   toks = tokens;
   tok = list_peek(toks, 0);
   idx = 0;
   len = list_size(toks);
   pt = tree_plant(NULL, 0);
   strcpy(pt->tag, "ROOT");

   // Construct the parse tree

   act = scene = line = NULL;
   //tok = NULL;

   parse_title();
   parse_dp(); // postprocess_dp();
   for (;;) {
      parse_act();
      for (;;) {
         parse_scene();
         for (;;) {
            switch (setjmp(parse_env)) {
               //case 0:           seek_stmt();    break;  /* longjmp */
               case NEXT_ENTER:  parse_enter();  break;
               case NEXT_EXIT:   parse_exit();   break;
               case NEXT_EXEUNT: parse_exeunt(); break;
               case NEXT_LINE:   parse_line();   break;  /* longjmp */
               case NEXT_SCENE:  goto EOS;
               case NEXT_ACT:    goto EOA;
               case NEXT_FINALE: goto EOE;
            }
            seek_stmt();
         }
         EOS:;
      }
      EOA:;
   }
   EOE:;

   list_destroy(toks);
   return pt;

   // static const stmthandler_t handlers[] = {
   //    { seek_act, parse_act },
   //    { seek_scene, parse_scene },
   //    { seek_enter, parse_enter },
   //    { seek_exit, parse_exit },
   //    { seek_exeunt, parse_exeunt },
   //    { seek_line, parse_line },
   //    { seek_asgn, parse_asgn },
   // };
   // static const int handlers_len = ARRLEN(handlers);
   // static stmthandler_t *handler;

   // while (idx < len) {
   //    reason = msgs.err.syn_eot;
   //    gettok();
   //    archive_tokstate(); // fixme: ttok하고 tidx 지역변수화?
   //    //printf("[%s]\n", tok->data);
   //    for (int i = 0; i < handlers_len; i++) {
   //       handler = handlers + i;
   //       if ((*handler->seek)()) {
   //          (*handler->parse)();
   //          goto next;
   //       }
   //       rewind_tokstate();
   //    }
   //    reason = msgs.err.syn_incomprehensible;
   //    synerr(&tell);
   //    next:;
   // }
}

static void parse_title(void) {
   tree_t *title;

   title = tree_graft(pt, NULL, 0, "TITLE");
   reason = msgs.err.syn_title_incomp;
   readtok('.', title);
}

static void parse_dp(void) {
   tree_t *dp, *character;
   //int ret;

   dp = tree_graft(pt, NULL, 0, "D.P.");
   for (;;) {
      reason = msgs.err.syn_dp_incomp;
      gettok();
      reason = msgs.err.syn_dp_noname;
      testtok(',');
      character = tree_graft(dp, NULL, 0, "CHAR");
      ungettok();
      reason = msgs.err.syn_dp_chardecl_incomp;
      readtok(',', character);
      reason = msgs.err.syn_dp_desc_incomp;
      skiptok('.');
      reason = msgs.err.syn_dp_nonext;
      gettok();
      //ret = strcmp(tok->data, "Act");
      if (seek_act()) break;
      ungettok();
   }
}

static void parse_act(void) {
   reason = msgs.err.syn_act_incomp;
   gettok();
   act = tree_graft(pt, tok->data, tok->dsiz, "ACT");
   /* use the same reason */
   gettok();
   reason = msgs.err.syn_act_badsyn;
   testtok(':');
   reason = msgs.err.syn_act_desc_incomp;
   skiptok('.');

   reason = msgs.err.syn_act_noscene;
   gettok();
}

static void parse_scene(void) {
   reason = msgs.err.syn_scene_noact;
  // validate(act);
   // the same structure with parse_act
   reason = msgs.err.syn_scene_incomp;
   gettok();
   scene = tree_graft(act, tok->data, tok->dsiz, "SCENE");
   /* use the same reason */
   gettok();
   reason = msgs.err.syn_scene_badsyn;
   testtok(':');
   reason = msgs.err.syn_scene_desc_incomp;
   skiptok('.');
}

// static int parse_stmt(void) {
//    if (idx == len)
//       return RET_EOE;

//    reason = msgs.err.syn_eot;
//    gettok();

//    archive_tokstate();
//    if (seek_line())
//       return RET_BOL;
//    rewind_tokstate();

//    for (int i = 0; i < hlen; i++) {
//       archive_tokstate();
//       enterlike = enterlikes + i;
//       if ((*enterlike->seek)())
//          return (*enterlike->parse)();
//       rewind_tokstate();
//    }

//    for (int i = 0; i < flen; i++) {
//       archive_tokstate();
//       finale = finales + i;
//       if ((*finale->seek)())
//          return finale->retval;
//       rewind_tokstate();
//    }

//    reason = msgs.err.syn_incomprehensible;
//    synerr(&tell);
//    return -1;  /* control never reaches here */
// }

static void parse_enter(void) {
   tree_t *enter, *character;

   reason = msgs.err.syn_enter_noscene;
   //validate(scene);

   enter = tree_graft(scene, NULL, 0, "ENTER");
   character = tree_graft(enter, NULL, 0, "CHAR");

   // Enter has 1 or 2 characters
   for (;;) {
      reason = msgs.err.syn_enter_incomp;
      gettok();
      if (tok->data[0] == ']')
         break;
      if (!strcmp(tok->data, "and")) {
         character = tree_graft(enter, NULL, 0, "CHAR");
         continue;
      }
      (void) tree_graft(
         character, tok->data, tok->dsiz, "CONTENT");
   }
   if (!enter->clen) {
      reason = msgs.err.syn_enter_nochar;
      synerr(&tell);
   }

   //return 1;
}

static void parse_exit(void) {
   tree_t *exit;

   reason = msgs.err.syn_exit_noscene;
   //validate(scene);

   exit = tree_graft(scene, NULL, 0, "EXIT");

   // Exit has 1 character
   reason = msgs.err.syn_exit_incomp;
   readtok(']', exit);

   if (!exit->clen) {
      reason = msgs.err.syn_exit_nochar;
      synerr(&tell);
   }

   //return 1;
}

static void parse_exeunt(void) {
   tree_t *exeunt, *character;

   reason = msgs.err.syn_exeunt_noscene;
   //validate(scene);

   exeunt = tree_graft(scene, NULL, 0, "EXEUNT");

   // Exeunt has either 0 or 2 characters
   for (;;) {
      reason = msgs.err.syn_exeunt_incomp;
      gettok();
      if (tok->data[0] == ']')
         break;
      if (!strcmp(tok->data, "and")) {
         character = tree_graft(exeunt, NULL, 0, "CHAR");
         continue;
      }
      (void) tree_graft(
         character, tok->data, tok->dsiz, "CONTENT");
   }
   if (exeunt->clen == 1) {
      reason = msgs.err.syn_exeunt_onechar;
      synerr(&tell);
   }

  // return 1;
}

static int seek_act(void) {
   return strcmp(tok->data, "Act") ? 0 : 1;
}

static int seek_scene(void) {
   return strcmp(tok->data, "Scene") ? 0 : 1;
}

static int seek_enter(void) {
   if (tok->data[0] != '[')
      return 0;
   gettok();
   return strcmp(tok->data, "Enter") ? 0 : 1;
}

static int seek_exit(void) {
   if (tok->data[0] != '[')
      return 0;
   gettok();
   return strcmp(tok->data, "Exit") ? 0 : 1;
}

static int seek_exeunt(void) {
   if (tok->data[0] != '[')
      return 0;
   gettok();
   return strcmp(tok->data, "Exeunt") ? 0 : 1;
}

static void postprocess_dp(void) {
   tree_t *dp, *character;
   char *name;
   int i, j, len;

   // len 이라는 이름 쓰면 헷갈릴수잇으니까 쓰지말기

   dp = pt->child[1];
   len = 0;

   for (i = 0; i < dp->clen; i++) {
      character = dp->child[i];
      if (character->clen == 1)
         continue;
      for (j = 0; j < character->clen; j++)
         len += character->child[j]->dsiz - 1;
      len++;  // \0
      name = malloc(len);
      name[0] = '\0';
      for (j = 0; j < character->clen; j++) {
         strcat(name, character->child[j]->data);
         tree_prune(character->child[i]);
      }
      (void) tree_graft(character, name, len, "CONTENT");
   }
}

static void gettok(void) {

   // if (!tok)
   //    tok = list_peek(toks, 0);
   // else
      tok = tok->next;


   if (!tok) synerr(&tell);
   idx++;
   etok = tok;
}

static void ungettok(void) {
   tok = tok->prev;
   idx--;
}

static void skiptok(char sentinel) {
   etok = tok;
   while (idx < len) {
      tok = list_peek(toks, idx++);
      if (strcmp(tok->tag, TOK_PNT))
         continue;
      if (tok->data[0] == sentinel)
         goto end;
   }
   synerr(&tell);
   end:;
}

static void testtok(char sentinel) {
   if (strcmp(tok->tag, TOK_PNT))
      return;
   if (tok->data[0] == sentinel)
      return;
   etok = tok;
   synerr(&tell);
}

static void readtok(char sentinel, tree_t *base) {
   etok = tok;
   while (idx < len) {
      tok = list_peek(toks, idx++);
      // if (strcmp(tok->tag, TOK_PNT))
      //    continue;
      if (tok->data[0] == sentinel)
      //if (match(tok->data[0], sentinels))
         goto end;
      (void) tree_graft(
         base, tok->data, tok->dsiz, "CONTENT");
   }
   synerr(&tell);
   end:;
}

static int match_tokdat(const char **arr, int len) {
   for (int i = 0; i < len; i++)
      if (!strcmp(tok->data, arr[i]))
         return i;
   return -1;
}

static inline void validate(tree_t *type) {
   if (type) return;
   etok = tok;
   synerr(&tell);
}

static void synerr(teller_t *tell) {
   //tok = tok->prev;
   //lnum = tok->lnum, lpos = tok->lpos;
   err_template(tell, Cbred, "<syntax error> ");
}

static void tell(void) {
   int lnum = etok->lnum, lpos = etok->lpos;
   line_t *l = ls + etok->lnum - 1;

   ffmtwrt(stderr,
      "%s\n"
      "[%s:%d:%d] " Cbwhite "note:" Creset " problematic since here\n"
      "%4d|%.*s" Cbblue "%s" Creset "\n",
      reason,
      sfname, lnum, lpos,
      lnum, lpos - 1, l->run, &l->run[lpos - 1]
   );
}

static inline void archive_tokstate(void) {
   //ttok = tok, tidx = idx;
   //reason = msgs.err.syn_eot;
   //gettok();
   ttok = tok, tidx = idx;
}

static inline void rewind_tokstate(void) {
   //tok = ttok, idx = tidx;
   tok = ttok, idx = tidx;
}

static void parse_const(tree_t *stmt) {
   static const char *decorators[] = {
      /* possessives */
      "my", "your", "thy", "thine",
      /* articles */
      "a", "an", "the"
   };
   static const int decorator_len
      = sizeof decorators / sizeof decorators[0];

   static bool cond1, cond2, cond3, cond4;
   static int ret;

   reason = msgs.err.syn_const_incomp;
   gettok();

   ret = match_tokdat(decorators, decorator_len);
   if (ret == 6) {  /* the */
      parse_op(stmt);
      return;
   }
   if (ret >= 0)
      gettok();  /* skips the decorator */

   for (;;) {


      // exit condition
      cond1 = match(tok->data[0], ".!?");
      cond2 = !strcmp(tok->data, "and");
      cond3 = !strcmp(tok->data, "not");
      cond4 = !strcmp(tok->data, "than");
      if (cond1 || cond2 || cond3)
         break;
      if (cond4) {
         ungettok(); ungettok();
         break;
      }

      // is it a decorator?
      if (match_tokdat(decorators, decorator_len) >= 0) {
         reason = msgs.err.syn_const_deco;
         synerr(&tell);
      }

      // is it an operator?
      // if (!strcmp(tok->data, "the")) {
      //    // parse_op(stmt);
      //    // continue;
      // }

      tree_graft(stmt, tok->data, tok->dsiz, "ADJ");

      reason = msgs.err.syn_const_incomp;
      gettok();
   }
   strcpy(stmt->child[stmt->clen - 1]->tag, "NOUN");

}

static void parse_op(tree_t *stmt) {
   static const char *operators[] = {
      /* <op> between ... */
      "difference", "quotient",
      /* <op> of ... */
      "sum", "product", "remainder", "cube",
      "square"
   };
   static const int operator_len
      = sizeof operators / sizeof operators[0];
   static int ret;

   ret = match_tokdat(operators, operator_len);
   if (ret == -1) {
      reason = msgs.err.syn_op_badop;
      synerr(&tell);
   }
   if (ret == 6) {
      reason = msgs.err.syn_op_incomp;
      gettok();
      if (strcmp(tok->data, "root")) {
         reason = msgs.err.syn_op_badsyn;
         synerr(&tell);
      }
      parse_op_operand(stmt, "of");
   }
   if (ret >= 2)
      parse_op_operand(stmt, "of");
   if (ret >= 0)
      parse_op_operand(stmt, "between");
}

static void parse_op_operand(tree_t *stmt, const char *type) {
   reason = msgs.err.syn_op_incomp;
   gettok();
   if (strcmp(tok->data, type)) {
      reason = msgs.err.syn_op_badsyn;
      synerr(&tell);
   }
   (void) parse_const(stmt);
   (void) parse_const(stmt);

}

static int seek_line(void) {
   //node_t *temp;


   // 이름비교하는로직

   tree_t *dp, *character;
   int i, k;

   dp = pt->child[1];

   //gettok();
   archive_tokstate();
   etok = tok;

   for (i = 0; i < dp->clen; i++) {
      character = dp->child[i];
      for (k = 0; k < character->clen; k++) {
         if (strcmp(character->child[k]->data, tok->data)) {
            //rewind_tokstate();
            break;
         }
         gettok();
      }
      rewind_tokstate();
      if (k == character->clen)
         goto name_found;
   }
   return 0;
   name_found:;
   //ungettok();
   //rewind_tokstate();
   return 1;


   // name:
   // if (!strcmp(tok->tag, TOK_PNT))
   //    return 0;
   //temp = tok;
//   gettok();
//   temp = tok;
   //skiptok(':');
   // if (tok->data[0] != ':')
   //    return 0;
   //tok = temp;
  // return 1;
   //return tok->data[0] == ':' ? 1 : 0;

}

static void parse_line(void) {
   static const stmthandler_t statements[] = {
      { seek_asgn, parse_asgn },
      { seek_out , parse_out  },
      { seek_in  , parse_in   },
      { seek_goto, parse_goto },
      { seek_cond, parse_cond },
      { seek_if  , parse_if   },
      { seek_push, parse_push },
      { seek_pop , parse_pop  }
   };
   static const int slen = ARRLEN(statements);

   const stmthandler_t *statement;
   tree_t *character;

   reason = msgs.err.syn_line_noscene;
   //validate(scene);

   //ungettok();
   line = tree_graft(scene, NULL, 0, "LINE");
   character = tree_graft(line, NULL, 0, "CHAR");
   ungettok();
   reason = msgs.err.syn_line_name_incomp;
   readtok(':', character);

   reason = msgs.err.syn_line_incomp;
   for (;;) {
      gettok();
      for (int i = 0; i < slen; i++) {
         archive_tokstate();
         statement = statements + i;
         if ((*statement->seek)()) {
            (*statement->parse)();
            break;
         }
         rewind_tokstate();
      }

      archive_tokstate();
      if (idx == len)    longjmp(parse_env, NEXT_FINALE);
      if (seek_line())   longjmp(parse_env, NEXT_LINE);
      if (seek_enter())  longjmp(parse_env, NEXT_ENTER);
      rewind_tokstate();
      if (seek_exit())   longjmp(parse_env, NEXT_EXIT);
      rewind_tokstate();
      if (seek_exeunt()) longjmp(parse_env, NEXT_EXEUNT);
      rewind_tokstate();
      if (seek_scene())  longjmp(parse_env, NEXT_SCENE);
      if (seek_act())    longjmp(parse_env, NEXT_ACT);

//      break;

      // reason = msgs.err.syn_line_nonext;
      // gettok();
      // for (int i = 0; i < hlen; i++) {
      //    archive_tokstate();
      //    enterlike = enterlikes + i;
      //    if ((*enterlike->seek)())
      //       return RET_EOT;
      //    rewind_tokstate();
      // }
      // for (int i = 0; i < flen; i++) {
      //    archive_tokstate();
      //    finale = finales + i;
      //    if ((*finale->seek)())
      //       return finale->retval;
      //    rewind_tokstate();
      // }
   }

   // reason = msgs.err.syn_incomprehensible;
   // synerr(&tell);
}

static void parse_line_as_consequent(void) {
   static const stmthandler_t statements[] = {
      { seek_asgn, parse_asgn },
      { seek_out , parse_out  },
      { seek_in  , parse_in   },
      { seek_goto, parse_goto },
      { seek_cond, parse_cond },
      { seek_if  , parse_if   },
      { seek_push, parse_push },
      { seek_pop , parse_pop  }
   };
   static const int slen = ARRLEN(statements);

   const stmthandler_t *statement;

   reason = msgs.err.syn_line_incomp;
   for (;;) {
      gettok();
      for (int i = 0; i < slen; i++) {
         archive_tokstate();
         statement = statements + i;
         if ((*statement->seek)()) {
            (*statement->parse)();
            break;
         }
         rewind_tokstate();
      }

      archive_tokstate();
      if (idx == len)    longjmp(parse_env, NEXT_FINALE);
      if (seek_line())   longjmp(parse_env, NEXT_LINE);
      if (seek_enter())  longjmp(parse_env, NEXT_ENTER);
      rewind_tokstate();
      if (seek_exit())   longjmp(parse_env, NEXT_EXIT);
      rewind_tokstate();
      if (seek_exeunt()) longjmp(parse_env, NEXT_EXEUNT);
      rewind_tokstate();
      if (seek_scene())  longjmp(parse_env, NEXT_SCENE);
      if (seek_act())    longjmp(parse_env, NEXT_ACT);
   }
}

static int seek_asgn(void) {
   char *ptr;

   ptr = &tok->data[0];
   *ptr = tolower(*ptr);
   if (strcmp(tok->data, "you") && strcmp(tok->data, "thou")) {
      *ptr = toupper(*ptr);
      return 0;
   }
   return 1;
}

static void parse_asgn(void) {
   int type;
   bool tcond1, tcond2, vcond1, vcond2;

   if (!strcmp(tok->data, "you"))
      type = 1;
   else  /* Thou */
      type = 2;

   reason = msgs.err.syn_asgn_incomp;
   gettok();

   tcond1 = type == 1;
   tcond2 = type == 2;
   vcond1 = !strcmp(tok->data, "are");
   vcond2 = !strcmp(tok->data, "art");

   if ((tcond1 && vcond2) || (tcond2 && vcond1)) {
      reason = msgs.err.syn_asgn_not_conj;
      synerr(&tell);
   }

   if (vcond1 || vcond2)
      parse_asgn_i();   // You are as ...
   else {
      ungettok();
      parse_asgn_ii();  // You ... sth!
   }
}

static void parse_asgn_i(void) {
   tree_t *asgn_i;

   reason = msgs.err.syn_asgn_incomp;
   gettok();
   if (strcmp(tok->data, "as")) {
      reason = msgs.err.syn_asgn_noas;
      synerr(&tell);
   }
   gettok();
   if (!strcmp(tok->tag, TOK_PNT)) {
      reason = msgs.err.syn_asgn_noadj;
      synerr(&tell);
   }
   asgn_i = tree_graft(line, tok->data, tok->dsiz, "ASSIGN");
   gettok();
   if (strcmp(tok->data, "as")) {
      reason = msgs.err.syn_asgn_noas;
      synerr(&tell);
   }
   parse_const(asgn_i);
}

static void parse_asgn_ii(void) {
   tree_t *asgn_ii;

   asgn_ii = tree_graft(line, NULL, 0, "ASSIGN");
   parse_const(asgn_ii);
}

static void seek_stmt(void) {
   if (idx == len)    longjmp(parse_env, NEXT_FINALE);

   reason = msgs.err.syn_eot;
   gettok();
   archive_tokstate();
   if (seek_enter())  longjmp(parse_env, NEXT_ENTER);
   rewind_tokstate();
   if (seek_exit())   longjmp(parse_env, NEXT_EXIT);
   rewind_tokstate();
   if (seek_exeunt()) longjmp(parse_env, NEXT_EXEUNT);
   rewind_tokstate();
   if (seek_line())   longjmp(parse_env, NEXT_LINE);
   rewind_tokstate();
   if (seek_scene())  longjmp(parse_env, NEXT_SCENE);
   if (seek_act())    longjmp(parse_env, NEXT_ACT);


   reason = msgs.err.syn_incomprehensible;
   synerr(&tell);
}

static int seek_out(void) {
   char *ptr;

   ptr = &tok->data[0];
   *ptr = tolower(*ptr);

   if (!strcmp(tok->data, "open")) {
      gettok(); gettok();
      if (strcmp(tok->data, "heart"))
         return 0;
      ungettok(); ungettok();
      return 1;
   }
   else
   if (!strcmp(tok->data, "speak"))
      return 1;
   else {
      *ptr = toupper(*ptr);
      return 0;
   }

   //return 1;
}

static void parse_out(void) {
   static char
      *out_num  = "OUT_NUM",
      *out_char = "OUT_CHAR";

   int type;
   node_t *tok1, *tok2;
   bool mcond, lcond1, lcond2;
   const char *tag;

   if (!strcmp(tok->data, "open"))
      type = 1, tag = out_num;
   else  /* Speak */
      type = 2, tag = out_char;

   reason = msgs.err.syn_out_incomp;
   gettok(); tok1 = tok;
   gettok(); tok2 = tok;

   mcond = !strcmp(tok1->data, "your") || !strcmp(tok1->data, "YOUR");
   lcond1 = !strcmp(tok2->data, "heart");
   lcond2 = !strcmp(tok2->data, "mind");

   if (!mcond) {
      reason = msgs.err.syn_out_badsyn;
      synerr(&tell);
   }
   if ((type == 1 && lcond2) || (type == 2 && lcond1)) {
      reason = msgs.err.syn_out_unmatched;
      synerr(&tell);
   }

   (void) tree_graft(line, NULL, 0, tag);

   gettok();
   if (!match(tok->data[0], ".!")) {
      reason = msgs.err.syn_out_badsyn;
      synerr(&tell);
   }
}

static int seek_in(void) {
   char *ptr;

   ptr = &tok->data[0];
   *ptr = tolower(*ptr);
   if (strcmp(tok->data, "listen") && strcmp(tok->data, "open")) {
      *ptr = toupper(*ptr);
      return 0;
   }
   return 1;
}

static void parse_in(void) {
   static char
      *in_num  = "IN_NUM",
      *in_char = "IN_CHAR";

   int type;
   node_t *tok1, *tok2;
   bool mcond, lcond1, lcond2;
   const char *tag;

   if (!strcmp(tok->data, "listen"))
      type = 1, tag = in_num;
   else  /* Open */
      type = 2, tag = in_char;

   reason = msgs.err.syn_in_incomp;
   if (type == 1) {
      gettok();
      if (strcmp(tok->data, "to")) {
         reason = msgs.err.syn_in_badsyn;
         synerr(&tell);
      }
   }
   gettok(); tok1 = tok;
   gettok(); tok2 = tok;

   mcond = !strcmp(tok1->data, "your") || !strcmp(tok1->data, "YOUR");
   lcond1 = !strcmp(tok2->data, "heart");
   lcond2 = !strcmp(tok2->data, "mind");

   if (!mcond) {
      reason = msgs.err.syn_in_badsyn;
      synerr(&tell);
   }
   if ((type == 1 && lcond2) || (type == 2 && lcond1)) {
      reason = msgs.err.syn_in_unmatched;
      synerr(&tell);
   }

   (void) tree_graft(line, NULL, 0, tag);

   gettok();
   if (!match(tok->data[0], ".!")) {
      reason = msgs.err.syn_in_badsyn;
      synerr(&tell);
   }
}

static int seek_goto(void) {
   char *ptr;

   ptr = &tok->data[0];
   *ptr = tolower(*ptr);
   if (strcmp(tok->data, "let") && strcmp(tok->data, "we")) {
      *ptr = toupper(*ptr);
      return 0;
   }
   return 1;
}

static void parse_goto(void) {
   int type;
   bool cond1, cond2;

   if (!strcmp(tok->data, "let"))
      type = 1;
   else  /* we */
      type = 2;

   reason = msgs.err.syn_goto_incomp;
   gettok();

   cond1 = !strcmp(tok->data, "us");
   cond2 = !strcmp(tok->data, "shall") || !strcmp(tok->data, "must");

   if (!cond1 && !cond2) {
      reason = msgs.err.syn_goto_badsyn;
      synerr(&tell);
   }
   if ((type == 1 && cond2) || (type == 2 && cond1)) {
      reason = msgs.err.syn_goto_unmatched;
      synerr(&tell);
   }

   gettok();

   cond1 = strcmp(tok->data, "return");
   cond2 = strcmp(tok->data, "proceed");

   if (cond1 && cond2) {
      reason = msgs.err.syn_goto_badsyn;
      synerr(&tell);
   }

   gettok();

   if (strcmp(tok->data, "to")) {
      reason = msgs.err.syn_goto_badsyn;
      synerr(&tell);
   }

   gettok();

   if (strcmp(tok->data, "Scene")) {
      reason = msgs.err.syn_goto_badsyn;
      synerr(&tell);
   }

   gettok();

   if (strcmp(tok->tag, TOK_TOK)) {
      reason = msgs.err.syn_goto_badsyn;
      synerr(&tell);
   }

   (void) tree_graft(line, tok->data, tok->dsiz, "GOTO");

   gettok();
   if (!match(tok->data[0], ".!")) {
      reason = msgs.err.syn_in_badsyn;
      synerr(&tell);
   }
}

static int seek_cond(void) {
   char *ptr;
   bool cond1, cond2, cond3, cond4;

   ptr = &tok->data[0];
   *ptr = tolower(*ptr);

   cond1 = strcmp(tok->data, "am");
   cond2 = strcmp(tok->data, "are");
   cond3 = strcmp(tok->data, "art");
   cond4 = strcmp(tok->data, "is");

   if (cond1 && cond2 && cond3 && cond4) {
      *ptr = toupper(*ptr);
      return 0;
   }
   return 1;
}

static void parse_cond(void) {
   static char
      *first = "first-person",
      *second = "second-person",
      *third = "third-person",
      *truthy = "true",
      *falsy = "false";

   int type;
   char *person, *truthval;
   bool cond1, cond2, cond3;
   tree_t *condition, *lefthand, *righthand;

   condition = tree_graft(line, NULL, 0, "QUESTION");
   lefthand = tree_graft(condition, NULL, 0, "LHS");
   righthand = tree_graft(condition, NULL, 0, "RHS");

   if (!strcmp(tok->data, "am"))
      type = 1, person = first;
   else
   if (!strcmp(tok->data, "are"))
      type = 2, person = second;
   else
   if (!strcmp(tok->data, "art"))
      type = 3, person = second;
   else
   if (!strcmp(tok->data, "is"))
      type = 4, person = third;

   reason = msgs.err.syn_cond_incomp;

   if (type < 4) {
      gettok();

      cond1 = !strcmp(tok->data, "I");
      cond2 = !strcmp(tok->data, "you");
      cond3 = !strcmp(tok->data, "thou");

      cond1 = type == 1 && !cond1;
      cond2 = type == 2 && !cond2;
      cond3 = type == 3 && !cond3;

      if (cond1 || cond2 || cond3) {
         reason = msgs.err.syn_cond_unmatched;
         synerr(&tell);
      }
   }
   else
      parse_const(condition);

   (void) tree_graft(
      lefthand, person, strlen(person) + 1, "PERSON");

   gettok();

   strcmp(tok->data, "not")
      ? (truthval = falsy)
      : (truthval = truthy);
   (void) tree_graft(
      condition, truthval, strlen(truthval) + 1, "NOT");

   gettok();

   if (!strcmp(tok->data, "as")) {
      gettok();

      if (strcmp(tok->data, TOK_TOK)) {
         reason = msgs.err.syn_cond_badsyn;
         synerr(&tell);
      }

      (void) tree_graft(
         condition, tok->data, tok->dsiz, "EQUAL");

      gettok();

      if (strcmp(tok->data, "as")) {
         reason = msgs.err.syn_cond_badsyn;
         synerr(&tell);
      }
   }
   else {
      (void) tree_graft(
         condition, tok->data, tok->dsiz, "COMPARE");

      gettok();

      if (type < 4 && strcmp(tok->data, "than")) {
         reason = msgs.err.syn_cond_badsyn;
         synerr(&tell);
      }
   }

   parse_const(righthand);
}

static int seek_if(void) {
   char *ptr;

   ptr = &tok->data[0];
   *ptr = tolower(*ptr);
   if (strcmp(tok->data, "if")) {
      *ptr = toupper(*ptr);
      return 0;
   }
   return 1;
}

static void parse_if(void) {
   static char
      *truthy = "true",
      *falsy = "false";

   char *truthval;
   tree_t *ifstmt;

   reason = msgs.err.syn_if_incomp;
   gettok();

   if (!strcmp(tok->data, "so"))
      truthval = truthy;
   else
   if (!strcmp(tok->data, "not"))
      truthval = falsy;
   else {
      reason = msgs.err.syn_if_badsyn;
      synerr(&tell);
   }

   ifstmt = tree_graft(line, NULL, 0, "IF");
   (void) tree_graft(ifstmt, truthval, strlen(truthval) + 1, "ANT");
   (void) tree_graft(ifstmt, NULL, 0, "CON");

   gettok();

   if (tok->data[0] != ',') {
      reason = msgs.err.syn_if_badsyn;
      synerr(&tell);
   }

   parse_line_as_consequent();
}

static int seek_push(void) {
   char *ptr;

   ptr = &tok->data[0];
   *ptr = tolower(*ptr);
   if (strcmp(tok->data, "remember")) {
      *ptr = toupper(*ptr);
      return 0;
   }
   return 1;
}

static void parse_push(void) {
   reason = msgs.err.syn_push_incomp;
   gettok();

   if (strcmp(tok->tag, TOK_TOK)) {
      reason = msgs.err.syn_push_badsyn;
      synerr(&tell);
   }

   (void) tree_graft(line, tok->data, tok->dsiz, "PUSH");

   gettok();
   if (!match(tok->data[0], ".!")) {
      reason = msgs.err.syn_in_badsyn;
      synerr(&tell);
   }

   // while (1) {
   //    reason = msgs.err.syn_in_badsyn;
   //    gettok();
   //    if (match(tok->data[0], ".!"))
   //       break;
   // }
}

static int seek_pop(void) {
   char *ptr;

   ptr = &tok->data[0];
   *ptr = tolower(*ptr);
   if (strcmp(tok->data, "recall")) {
      *ptr = toupper(*ptr);
      return 0;
   }
   return 1;
}

static void parse_pop(void) {
   (void) tree_graft(line, NULL, 0, "POP");
   reason = msgs.err.syn_pop_incomp;
   while (1) {
      gettok();
      if (match(tok->data[0], ".!?"))
         break;
   }
}
