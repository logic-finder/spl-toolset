#include "parse.h"
#include "parse.type.h"

/*
 - p: line number
 - q: position in the line
 - They are used in the 'parse_' prefixed functions.
 - Those function are called in sequence and no other
   function uses p and q.
 - t = temporary
*/

/******************************
 * IMPORTANT GLOBAL VARIABLES *
 ******************************/
static line_t *l;   // line
static line_t *tl;  // temp. var. for l

static int p;   // line number
static int q;   // position in line
static int tp;  // temp. var. for p
static int tq;  // temp. var. for q

static char ch;
static char *buf;
static int i, max;

extern void parse(optflg_t *of, optval_t *ov, line_t *lines, int cnt, tree_t *pt) {
   // initializes global variables
   p = q = 0;
   l = &lines[p];
   max = 128;
   buf = smalloc(max);

   // analyzes the contents
   int ret;

   parse_title(lines, cnt, pt);
   parse_dp(lines, cnt, pt);
   for (;;) {
      tree_t *act
         = parse_act(lines, cnt, pt);
      for (;;) {
         tree_t *scene
            = parse_scene(lines, cnt, act);
         for (;;) {
            ret = parse_stmt(lines, cnt, scene);
            switch (ret) {
               case RET_EOT: goto EOT;
               case RET_EOS: goto EOS;
               case RET_EOA: goto EOA;
               case RET_EOE: goto EOE;
            }
            EOT:;
         }
         EOS:;
         // EOS: ret = seek_scene(lines, cnt);
         // // tell_scene(ret);
      }
      EOA:;// ret = seek_act(lines, cnt);
      // tell_act(ret);
   }
   EOE:;

   // cleanup
   free(buf);
}

static void traverse(tree_t *t) {
   char *x;

   if (t->dsiz == 0) x = "no data";
   else x = (char *) t->data;

   printf("%s: %s\n", t->tag, x);

   if (t->clen == 0)
      return;

   for (int i = 0; i < t->clen; i++)
      traverse(t->child[i]);
}

static void parse_title(line_t *lines, int cnt, tree_t *pt) {
   // skips preceding whitespaces
   while (p < cnt) {
      while (q < l->len) {
         if (isspace(l->run[q++]))
            continue;
         q--;
         goto after_pws;
      }
      q = 0;
      l = &lines[++p];
   }
   ERR("syntax error: no title present.");
   after_pws:;

   // stores the title
   i = 0;
   while (p < cnt) {
      while (q < l->len) {
         ch = l->run[q++];
         if (ch == '.')
            goto end_of_title;
         buf[i++] = ch;
         if (i == max) {
            max *= 2;
            buf = srealloc(buf, max);
         }
      }
      q = 0;
      l = &lines[++p];
   }
   ERR("syntax error: EOF encountered while reading title.");
   end_of_title:;

   if (!isspace(buf[i - 1]))
      buf[i] = '\0';  /* there must be a room */
   else
   /* in order to remove trailing spaces */
      for (int idx = i - 1; idx >= 0 ; idx--) {
         if (isspace(buf[idx]))
            continue;
         buf[idx + 1] = '\0';
      }

   // constructs pt
   tree_t *title;

   title = tree_plant(buf, i + 1);
   strcpy(title->tag, "title");
   tree_addchild(pt, title);
}

static int seek_act(line_t *lines, int cnt);
static void resetpos(void);

static void parse_dp(line_t *lines, int cnt, tree_t *pt) {
   // skips preceding whitespaces
   while (p < cnt) {
      while (q < l->len) {
         if (isspace(l->run[q++]))
            continue;
         q--;
         goto after_pws;
      }
      q = 0;
      l = &lines[++p];
   }
   ERR("syntax error: no dramatis personae present.");
   after_pws:;

   // dp has names as its children
   tree_t *dp;

   dp = tree_plant(NULL, 0);
   strcpy(dp->tag, "dp");

   // stores characters
   tree_t *name;

   for (;;) {
      tl = l;
      i = 0;
      while (p < cnt) {
         while (q < l->len) {
            ch = l->run[q++];
            if (ch == ',')
               goto end_of_name;
            buf[i++] = ch;
            if (i == max) {
               max *= 2;
               buf = srealloc(buf, max);
            }
         }
         q = 0;
         l = &lines[++p];
      }
      VERR("syntax error: name not finished\n"
         "%4d|%s", tl->num, tl->run);
      end_of_name: buf[i] = '\0';

      // constructs dp
      name = tree_plant(buf, i + 1);
      strcpy(name->tag, "chardecl");
      tree_addchild(dp, name);

      // skips description
      while (p < cnt) {
         while (q < l->len) {
            if (l->run[q++] != '.')
               continue;
            goto end_of_desc;
         }
         q = 0;
         l = &lines[++p];
      }
      l = lines + --p;
      VERR("\033[0;31m<syntax error>\033[0m end of source file\n"
         "context: on the description of the character \033[0;33m%s\033[0m\n"
         "%4d|%s\033[0;35m<EOF>\033[0m",
         buf, l->num, l->run);
      end_of_desc:;

      // skips whitespaces
      while (p < cnt) {
         while (q < l->len) {
            if (isspace(l->run[q++]))
               continue;
            q--;
            goto after_ws;
         }
         q = 0;
         l = &lines[++p];
      }
      VERR("syntax error: EOF encountered after the declaration of the character %s.", buf);
      after_ws:;

      /*
      if the next statement is not "Act ...:",
      it means the dramatis personae is not ended.
      thus, reads it as a character declaration.
      */
      int ret;

      ret = seek_act(lines, cnt);
      if (ret) {
         resetpos();
         continue;
      }
      break;  /* end of dramatis personae */
   }

   // constructs pt
   tree_addchild(pt, dp);
}

static int seek_actlike(line_t *lines, int cnt, const char *type);

static int seek_act(line_t *lines, int cnt) {
   return seek_actlike(lines, cnt, "Act");
}

static int seek_actlike(line_t *lines, int cnt, const char *type) {
   // inits temps
   tp = p;
   tq = q;
   tl = l;

   // stores 3 or 5 letters for comparison to "Act" or "Scene"
   const int typelen = strlen(type);

   i = 0;
   while (p < cnt) {
      while (q < l->len) {
         if (i == typelen) {
            buf[i] = '\0';
            goto before_type_test;
         }
         buf[i++] = l->run[q++];
      }
      q = 0;
      l = lines + ++p;
   }
   l = lines + --p;
   VERR("<syntax error> end of source file\n"
      "%4d|%s", l->num, l->run);
   before_type_test:;

   // if buf doesn't begin with "Act" or "Scene"
   if (strcmp(buf, type))
      return 1;

   // stores one letter for comp. to a whitespace
   i = 0;
   while (p < cnt) {
      while (q < l->len) {
         if (i == 1) {
            buf[i] = '\0';
            goto before_ws_test;
         }
         buf[i++] = l->run[q++];
      }
      q = 0;
      l = lines + ++p;
   }
   l = lines + --p;
   VERR("<syntax error> end of source file\n"
      "%4d|%s", l->num, l->run);
   before_ws_test:;

   // if buf[0] isn't a whitespace
   if (!isspace(buf[0]))
      return 2;

   // skips whitespaces, if any.
   while (p < cnt) {
      while (q < l->len) {
         if (isspace(l->run[q++]))
            continue;
         q--;
         goto before_roman_num_read;
      }
      q = 0;
      l = lines + ++p;
   }
   l = lines + --p;
   VERR("<syntax error> end of source file\n"
      "%4d|%s", l->num, l->run);
   before_roman_num_read:;

   // stores letters for comp. to roman numerals
   i = 0;
   while (p < cnt) {
      while (q < l->len) {
         ch = l->run[q++];
         if (!match(ch, ROMAN_NUM)) {
            q--;
            buf[i] = '\0';
            goto before_ws_skip;
         }
         buf[i++] = ch;
      }
      q = 0;
      l = lines + ++p;
   }
   l = lines + --p;
   VERR("<syntax error> end of source file\n"
      "%4d|%s", l->num, l->run);
   before_ws_skip:;

   // skips whitespace, if any.
   while (p < cnt) {
      while (q < l->len) {
         if (isspace(l->run[q++]))
            continue;
         q--;
         goto before_colon_test;
      }
      q = 0;
      l = lines + ++p;
   }
   l = lines + --p;
   VERR("<syntax error> end of source file\n"
      "%4d|%s", l->num, l->run);
   before_colon_test:;

   // if not colon
   if (l->run[q] != ':')
      return 3;
   return 0;
}

static void resetpos(void) {
   p = tp;
   q = tq;
   l = tl;
}

static tree_t *parse_act(line_t *lines, int cnt, tree_t *pt) {
   // int ret;

   // ret = seek_act(lines, cnt);
   // switch (ret) {
   //    case 0: break;
   //    case 1: VERR("<syntax error> Act not found\n"
   //       "%4d|%s", l->num, l->run); break;
   //    case 2: VERR("<syntax error> no whitespace follows Act\n"
   //       "%4d|%s", l->num, l->run); break;
   //    case 3: VERR("<syntax error> no colon follows Act\n"
   //       "%4d|%s", l->num, l->run); break;
   // }

   // skips description
   while (p < cnt) {
      while (q < l->len) {
         if (l->run[q++] != '.')
            continue;
         goto end_of_desc;
      }
      q = 0;
      l = lines + ++p;
   }
   l = lines + --p;
   VERR("<syntax error> end of source file\n"
      "context: on the description of Act %s\n"
      "%4d|%s<EOF>", buf, l->num, l->run);
   end_of_desc:;

   // skips whitespaces
   while (p < cnt) {
      while (q < l->len) {
         if (isspace(l->run[q++]))
            continue;
         q--;
         goto after_ws;
      }
      q = 0;
      l = lines + ++p;
   }
   l = lines + --p;
   VERR("<syntax error> end of source file\n"
      "context: after the description of Act %s\n"
      "%4d|%s<EOF>", buf, l->num, l->run);
   after_ws:;

   tree_t *act;

   // buf contains roman numerals.
   act = tree_plant(buf, i + 1);
   strcpy(act->tag, "act");
   tree_addchild(pt, act);

   return act;
}

static int seek_scene(line_t *lines, int cnt);

static tree_t *parse_scene(line_t *lines, int cnt, tree_t *act) {
   int ret;

   ret = seek_scene(lines, cnt);
   switch (ret) {
      case 0: break;
      case 1: VERR("<syntax error> Scene not found\n"
         "%4d|%s", l->num, l->run); break;
      case 2: VERR("<syntax error> no whitespace follows Scene\n"
         "%4d|%s", l->num, l->run); break;
      case 3: VERR("<syntax error> no colon follows Scene\n"
         "%4d|%s", l->num, l->run); break;
   }

   // skips description
   while (p < cnt) {
      while (q < l->len) {
         if (l->run[q++] != '.')
            continue;
         goto end_of_desc;
      }
      q = 0;
      l = lines + ++p;
   }
   l = lines + --p;
   VERR("<syntax error> end of source file\n"
      "context: on the description of Scene %s\n"
      "%4d|%s<EOF>", buf, l->num, l->run);
   end_of_desc:;

   // skips trailing whitespaces
   while (p < cnt) {
      while (q < l->len) {
         if (isspace(l->run[q++]))
            continue;
         q--;
         goto after_ws;
      }
      q = 0;
      l = lines + ++p;
   }
   l = lines + --p;
   VERR("<syntax error> end of source file\n"
      "context: after the description of Scene %s\n"
      "%4d|%s<EOF>", buf, l->num, l->run);
   after_ws:;

   tree_t *scene;

   // buf contains roman numerals.
   scene = tree_plant(buf, i + 1);
   strcpy(scene->tag, "scene");
   tree_addchild(act, scene);

   return scene;
}

static int seek_scene(line_t *lines, int cnt) {
   return seek_actlike(lines, cnt, "Scene");
}

static int seek_enter(line_t *lines, int cnt);
static int seek_exit(line_t *lines, int cnt);
static int seek_exeunt(line_t *lines, int cnt);
static int seek_line(line_t *lines, int cnt);
static int parse_enter(line_t *lines, int cnt, tree_t *scene);
static int parse_exit(line_t *lines, int cnt, tree_t *scene);
static int parse_exeunt(line_t *lines, int cnt, tree_t *scene);
static int parse_line(line_t *lines, int cnt, tree_t *scene);
// static void tell_enter(int ret);
// static void tell_exit(int ret);
// static void tell_exeunt(int ret);

static int parse_stmt(line_t *lines, int cnt, tree_t *scene) {
   if (p == cnt)
      return RET_EOE;

   static const stmt_t stmts[] = {
      { seek_enter , parse_enter  },
      { seek_exit  , parse_exit   },
      { seek_exeunt, parse_exeunt },
      { seek_line  , parse_line   }
   };
   const int len = sizeof stmts / sizeof stmts[0];
   int i, ret;
   const stmt_t *stmt;

   for (i = 0; i < len; i++) {
      stmt = stmts + i;
      ret = stmt->seeker(lines, cnt);
      if (!ret)
         goto token_found;
      resetpos();
   }

   if (seek_scene(lines, cnt))
      resetpos();
   else
      return RET_EOS;

   if (seek_act(lines, cnt))
      resetpos();
   else
      return RET_EOA;

   // VERR("<syntax error> incomprehensible token\n"
   //    "%4d|%s", l->num, l->run);
   ffmtwrt(stderr, "\033[0;31m<syntax error>\033[0m incomprehensible token\n");
   while (1) {
      if (strlen(&l->run[q]) - 1 == 0) {
         q = 0;
         l = lines + ++p;
      }
      else
         break;
   }
   ffmtwrt(stderr, "%4d|%.*s", l->num, q, l->run);
   ffmtwrt(stderr, "\033[0;35m%.*s\033[0m\n", strlen(&l->run[q]) - 1, &l->run[q]);
   ffmtwrt(stderr, "    |");
   if (q > 0) ffmtwrt(stderr, "%*c", q, ' ');
   ffmtwrt(stderr, "\033[0;35m^%.*s\033[0m\n", strlen(&l->run[q]) - 2, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
   exit(1);
      // "%4d|\033[0;35m%s\033[0m"
      // "    |\033[0;35m^%.*s\033[0m", l->num, &l->run[q], strlen(&l->run[q]) - 2,
      // "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

   token_found:
   return stmt->parser(lines, cnt, scene);
}

static bool eos(int cnt);
static int seek_enterlike(line_t *lines, int cnt, const char *type);

static int seek_enter(line_t *lines, int cnt) {
   return seek_enterlike(lines, cnt, "Enter");
}

static int seek_exit(line_t *lines, int cnt) {
   return seek_enterlike(lines, cnt, "Exit");
}

static int seek_exeunt(line_t *lines, int cnt) {
   return seek_enterlike(lines, cnt, "Exeunt");
}

static int seek_enterlike(line_t *lines, int cnt, const char *type) {
   // inits temps
   tp = p; tq = q; tl = l;

   // stores one letter for comp. to a '['
   i = 0;
   while (p < cnt) {
      while (q < l->len) {
         if (i == 1) {
            buf[i] = '\0';
            goto before_bracket_check;
         }
         buf[i++] = l->run[q++];
      }
      q = 0;
      l = lines + ++p;
   }
   l = lines + --p;
   VERR("<syntax error> end of source file\n"
      "%4d|%s", l->num, l->run);
   before_bracket_check:;

   // starts with '['?
   if (buf[0] != '[')
      return 1;   /* [ is followed by Enter, Exit, or Exeunt */

   // skips whitespaces, if any.
   while (p < cnt) {
      while (q < l->len) {
         if (isspace(l->run[q++]))
            continue;
         q--;
         goto before_type_read;
      }
      q = 0;
      l = lines + ++p;
   }
   l = lines + --p;
   VERR("<syntax error> end of source file\n"
      "%4d|%s", l->num, l->run);
   before_type_read:;

   // stores "Enter", "Exit", or "Exeunt" depending on 'type'
   const int typelen = strlen(type);

   i = 0;
   while (p < cnt) {
      while (q < l->len) {
         if (i == typelen) {
            buf[i] = '\0';
            goto before_type_test;
         }
         buf[i++] = l->run[q++];
      }
      q = 0;
      l = lines + ++p;
   }
   l = lines + --p;
   VERR("<syntax error> end of source file\n"
      "%4d|%s", l->num, l->run);
   before_type_test:;

   // if buf doesn't begin with Enter, Exit, or Exeunt
   if (strcmp(buf, type))
      return 2;
   return 0;
}

static bool eos(int cnt) {
   // this function doesn't manipulate global variables
   return p < cnt ? false : true;
}

static int parse_enter(line_t *lines, int cnt, tree_t *scene) {
   // stores one letter for comp. to a whitespace
   i = 0;
   while (p < cnt) {
      while (q < l->len) {
         if (i == 1) {
            buf[i] = '\0';
            goto before_ws_test;
         }
         buf[i++] = l->run[q++];
      }
      q = 0;
      l = lines + ++p;
   }
   l = lines + --p;
   VERR("<syntax error> end of source file\n"
      "context: after Enter\n"
      "%4d|%s", l->num, l->run);
   before_ws_test:;

   if (buf[0] == ']') {
      //l = lines + --p;
      VERR("<syntax error> no character specified\n"
         "context: in this Enter\n"
         "%4d|%s", l->num, l->run);
   }
   if (!isspace(buf[0])) {
      //l = lines + --p;
      VERR("<syntax error> no whitespace follows Enter\n"
         "%4d|%s", l->num, l->run);
   }

   // skips whitespaces, if any.
   // while (p < cnt) {
   //    while (q < l->len) {
   //       if (isspace(l->run[q++]))
   //          continue;
   //       q--;
   //       goto after_ws;
   //    }
   //    q = 0;
   //    l = lines + ++p;
   // }
   // l = lines + --p;
   // VERR("<syntax error> end of source file\n"
   //    "context: in the middle of Enter\n"
   //    "%4d|%s", l->num, l->run);
   // after_ws:;

   /*
   - reads a character name and stores it
   - multiple characters can be stated in Enter
   - for example: [Enter A, B, and C]
   - our strategy is the following:
      1. takes everything before ']', since
         we've already passed "[Enter ".
      2. reduces a sequence of whitespaces to a single ' '
      3. splits against ','
      4. removes preceding & trailing spaces of each element
      5. stores them!
         5.1. splits the first element having " and " into two
   */

   // Phase I
   i = 0;
   while (p < cnt) {
      while (q < l->len) {
         ch = l->run[q++];
         if (ch == ']') {
            buf[i] = '\0';
            goto before_list_process;
         }
         buf[i++] = ch;
         if (i == max) {
            max *= 2;
            buf = srealloc(buf, max);
         }
      }
      q = 0;
      l = lines + ++p;
   }
   l = lines + --p;
   VERR("<syntax error> end of source file\n"
      "context: in this Enter\n"
      "%4d|%s", l->num, l->run);
   before_list_process:;

   if (!strlen(buf))
      VERR("<syntax error> no character specified\n"
         "context: in this Enter\n"
         "%4d|%s", l->num, l->run);

   // Phase II
    char *ret;//, *pos;

   // ret = reducews(buf);
   // if (!strlen(ret))
   //    VERR("<syntax error> no character specified\n"
   //       "context: in this Enter\n"
   //       "%4d|%s", l->num, l->run);

   // Phase III
   // pos = strstr(ret, " and");
   // if (pos) {
   //    *pos = '\0';
   //    strcpy(buf, ret);
   //    strcat(buf, pos + 4);
   // }
   // else strcpy(buf, ret);
   // free(ret);

   // Phase IV
   int siz;
   char **arr;

   arr = split(buf, ",", &siz);
   //arr = split(ret, ",", &siz);
   //free(ret);

   // for (int v = 0; v < siz; v++) {
   //    trim(arr[v]);
   //    ret = strstr(arr[v], " and ");
   //    if (!ret)
   //       continue;


   // }

   // Phase V + VI and cleanup
   tree_t *enter_stmt, *enter_char;
   bool first_and;
   char *name;
   char *and = " and ";

   enter_stmt = tree_plant(NULL, 0);
   strcpy(enter_stmt->tag, "enter");

   first_and = false;
   for (int v = 0; v < siz; v++) {
      name = arr[v];
      if (!first_and) {
         ret = strstr(name, and);
         if (!ret)
            goto below;
         *ret = '\0';
         trim(name);
         enter_char = tree_plant(name, strlen(name) + 1);
         strcpy(enter_char->tag, "enter char");
         tree_addchild(enter_stmt, enter_char);
         name = ret + strlen(and);
         first_and = true;
      }
      below:;
      trim(name);
      enter_char = tree_plant(name, strlen(name) + 1);
      strcpy(enter_char->tag, "enter char");
      tree_addchild(enter_stmt, enter_char);
      free(arr[v]);
   }
   free(arr);
   tree_addchild(scene, enter_stmt);

   // skips trailing whitespaces
   while (p < cnt) {
      while (q < l->len) {
         if (isspace(l->run[q++]))
            continue;
         q--;
         goto after_ws;
      }
      q = 0;
      l = &lines[++p];
   }
   // 그냥 l 1빼기 하면 안되는거 알지?
   ERR("syntax error: end of source file\n"
      "context: after this Enter");
   after_ws:;

   return RET_EOT;
}

static int parse_exit(line_t *lines, int cnt, tree_t *scene) {
   // stores one letter for comp. to a whitespace
   i = 0;
   while (p < cnt) {
      while (q < l->len) {
         if (i == 1) {
            buf[i] = '\0';
            goto before_ws_test;
         }
         buf[i++] = l->run[q++];
      }
      q = 0;
      l = lines + ++p;
   }
   l = lines + --p;
   VERR("<syntax error> end of source file\n"
      "context: after Exit\n"
      "%4d|%s", l->num, l->run);
   before_ws_test:;

   if (buf[0] == ']')
      VERR("\033[0;31m<syntax error>\033[0m no character specified\n"
         "context: in this \033[0;33mExit\033[0m\n"
         "%4d|%s", l->num, l->run);
   if (!isspace(buf[0]))
      VERR("<syntax error> no whitespace follows Exit\n"
         "%4d|%s", l->num, l->run);

   // skips whitespaces, if any.
   // while (p < cnt) {
   //    while (q < l->len) {
   //       if (isspace(l->run[q++]))
   //          continue;
   //       q--;
   //       goto after_ws;
   //    }
   //    q = 0;
   //    l = lines + ++p;
   // }
   // l = lines + --p;
   // VERR("<syntax error> end of source file\n"
   //    "context: in the middle of Exit\n"
   //    "%4d|%s", l->num, l->run);
   // after_ws:;

   // reads letters until ']'
   i = 0;
   while (p < cnt) {
      while (q < l->len) {
         ch = l->run[q++];
         if (ch == ']') {
            buf[i] = '\0';
            goto before_char_process;
         }
         buf[i++] = ch;
         if (i == max) {
            max *= 2;
            buf = srealloc(buf, max);
         }
      }
      q = 0;
      l = lines + ++p;
   }
   l = lines + --p;
   VERR("<syntax error> end of source file\n"
      "context: in this Exit\n"
      "%4d|%s", l->num, l->run);
   before_char_process:;

   if (!strlen(buf))
      VERR("<syntax error> no character specified\n"
         "context: in this Exit\n"
         "%4d|%s", l->num, l->run);

   //char *ret;
  // int len;

   //ret = reducews(buf);
   // len = strlen(ret);
   // if (!len)
   //    VERR("<syntax error> no character specified\n"
   //       "context: in this Exit\n"
   //       "%4d|%s", l->num, l->run);

   tree_t *exit_stmt, *exit_char;

   exit_stmt = tree_plant(NULL, 0);
   strcpy(exit_stmt->tag, "exit");
   trim(buf);
   exit_char = tree_plant(buf, strlen(buf) + 1);
   //exit_char = tree_plant(ret, len + 1);
   strcpy(exit_char->tag, "exit char");
   //free(ret);
   tree_addchild(exit_stmt, exit_char);
   tree_addchild(scene, exit_stmt);

   // skips trailing whitespaces
   while (p < cnt) {
      while (q < l->len) {
         if (isspace(l->run[q++]))
            continue;
         q--;
         goto after_ws;
      }
      q = 0;
      l = &lines[++p];
   }
   after_ws:;

   return RET_EOT;
}

static int parse_exeunt(line_t *lines, int cnt, tree_t *scene) {
   // stores one letter for comp. to a ws or ']'
   i = 0;
   while (p < cnt) {
      while (q < l->len) {
         if (i == 1) {
            buf[i] = '\0';
            goto before_char_test;
         }
         buf[i++] = l->run[q++];
      }
      q = 0;
      l = lines + ++p;
   }
   l = lines + --p;
   VERR("<syntax error> end of source file\n"
      "context: after Exeunt\n"
      "%4d|%s", l->num, l->run);
   before_char_test:;

   // if no char specified, return.
   tree_t *exeunt_stmt;

   if (buf[0] == ']') {
      exeunt_stmt = tree_plant(NULL, 0);
      strcpy(exeunt_stmt->tag, "exeunt");
      return RET_EOT;
   }

   // neither a w.s. nor ]
   if (!isspace(buf[0]))
      VERR("<syntax error> no whitespace\n"
         "context: between this Exeunt and a name\n"
         "%4d|%s", l->num, l->run);



   // Phase I
   i = 0;
   while (p < cnt) {
      while (q < l->len) {
         ch = l->run[q++];
         if (ch == ']') {
            buf[i] = '\0';
            goto before_list_process;
         }
         buf[i++] = ch;
         if (i == max) {
            max *= 2;
            buf = srealloc(buf, max);
         }
      }
      q = 0;
      l = lines + ++p;
   }
   l = lines + --p;
   VERR("<syntax error> end of source file\n"
      "context: in this Exeunt\n"
      "%4d|%s", l->num, l->run);
   before_list_process:;

   if (!strlen(buf)) {
      exeunt_stmt = tree_plant(NULL, 0);
      strcpy(exeunt_stmt->tag, "exeunt");
      return RET_EOT;
   }

   // Phase II
   char *ret;//, *pos;

   //ret = reducews(buf);
   // if (!strlen(ret)) {
   //    exeunt_stmt = tree_plant(NULL, 0);
   //    strcpy(exeunt_stmt->tag, "exeunt");
   //    return RET_EOT;
   // }

   int siz;
   char **arr;

   //arr = split(ret, ",", &siz);
   arr = split(buf, ",", &siz);
   //free(ret);

   tree_t *exeunt_char;
   bool first_and;
   char *name;
   char *and = " and ";

   exeunt_stmt = tree_plant(NULL, 0);
   strcpy(exeunt_stmt->tag, "exeunt");

   first_and = false;
   for (int v = 0; v < siz; v++) {
      name = arr[v];
      if (!first_and) {
         ret = strstr(name, and);
         if (!ret)
            goto below;
         *ret = '\0';
         trim(name);
         exeunt_char = tree_plant(name, strlen(name) + 1);
         strcpy(exeunt_char->tag, "exeunt char");
         tree_addchild(exeunt_stmt, exeunt_char);
         name = ret + strlen(and);
         first_and = true;
      }
      below:;
      trim(name);
      exeunt_char = tree_plant(name, strlen(name) + 1);
      strcpy(exeunt_char->tag, "exeunt char");
      tree_addchild(exeunt_stmt, exeunt_char);
      free(arr[v]);
   }

   if (exeunt_stmt->clen < 2) {
      VERR("<syntax error> only one character specified\n"
         "context: in this Exeunt\n"
         "%4d|%s", l->num, l->run);
   }

   free(arr);
   tree_addchild(scene, exeunt_stmt);

   // skips trailing whitespaces
   while (p < cnt) {
      while (q < l->len) {
         if (isspace(l->run[q++]))
            continue;
         q--;
         goto after_ws;
      }
      q = 0;
      l = &lines[++p];
   }
   after_ws:;

   return RET_EOT;
}

static int seek_line(line_t *lines, int cnt) {
   // inits temps
   tp = p, tq = q, tl = l;

   // stores letters until ':'
   i = 0;
   while (p < cnt) {
      while (q < l->len) {
         ch = l->run[q++];
         if (ch == ':') {
            buf[i] = '\0';
            goto end_of_name;
         }
         buf[i++] = ch;
         if (i == max) {
            max *= 2;
            buf = srealloc(buf, max);
         }
      }
      q = 0;
      l = lines + ++p;
   }
   l = lines + --p;
   VERR("<syntax error> end of source file\n"
      "%4d|%s", l->num, l->run);
   end_of_name:;

   // skips whitespaces, if any.
   while (p < cnt) {
      while (q < l->len) {
         if (isspace(l->run[q++]))
            continue;
         q--;
         goto after_ws;
      }
      q = 0;
      l = &lines[++p];
   }
   VERR("<syntax error> end of source file\n"
      "context: on the line of %s\n"
      "%4d|%s", buf, l->num, l->run);
   after_ws:;

   return 0;
}

static int seek_asgn_i(line_t *lines, int cnt);
static int seek_asgn_ii(line_t *lines, int cnt);
static int parse_asgn_i(line_t *lines, int cnt, tree_t *scene);
static int parse_asgn_ii(line_t *lines, int cnt, tree_t *scene);

static int parse_line(line_t *lines, int cnt, tree_t *scene) {
   // // inits temps
   // tp = p, tq = q, tl = l;

   // // assignment statement?
   // i = 0;

   static const stmt_t stmts[] = {
      { seek_asgn_i , parse_asgn_i  },
      { seek_asgn_ii, parse_asgn_ii }
   };
   const int len = sizeof stmts / sizeof stmts[0];
   int i, ret;
   const stmt_t *stmt;

   for (i = 0; i < len; i++) {
      stmt = stmts + i;
      ret = stmt->seeker(lines, cnt);
      if (!ret)
         goto token_found;
      resetpos();
   }

   ERR("shit things gone astray");

   token_found:
   return stmt->parser(lines, cnt, scene);
}

