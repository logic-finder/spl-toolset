#include "typecheck.h"
#include "typecheck.type.h"

static const node_t *enode;
static tree_t *pt;
static bool ret;
static int val;

extern void typecheck(optflg_t *of, optval_t *ov, tree_t *_pt) {
   // Initialize
   pt = _pt;

   // Preprocess names
   tree_post_traverse(pt, coalesce_name, 0);

   // Type-check
   tree_post_traverse(pt, typecheck_router, 0);
}

static void coalesce_name(tree_t *t, int _) {
   node_t *n;
   char *buf;
   int i, clen, rlen;

   (void) _;
   n = tree_dat(t);
   if (strcmp(n->tag, "CHAR"))
      return;

   clen = tree_clen(t);
   rlen = 0;
   for (i = 0; i < clen; i++)
      // notice that len == strlen(run) + 1
      rlen += TREE_CHDAT(t, i)->len;

   /*
    * input  = ["the", "misty", "Ghost"]
    * output = "the_misty_Ghost00"
    *    where _ = space and 0 = \0
    */
   rlen++;
   buf = smalloc(rlen);
   buf[0] = '\0';
   for (i = 0; i < clen; i++) {
      strcat(buf, TREE_CHDAT(t, i)->run);
      strcat(buf, " ");
   }
   buf[rlen - 2] = '\0';

   n->run = buf;
   n->len = rlen;
}

static void typecheck_router(tree_t *t, int _) {
   typedef struct typehandler {
      const char *kind;
      typechecker_t *check;
   } typehandler_t;

   static const typehandler_t types[] = {
      { "ADJ"     , typecheck_adj  },
      { "NOUN"    , typecheck_noun },
      { "CHAR"    , typecheck_char },
      { "ASSIGN"  , typecheck_adj  },
      { "EQUAL"   , typecheck_adj  },
      { "COMPARE" , typecheck_comp },
      { "ACT"     , typecheck_rnum },
      { "SCENE"   , typecheck_rnum },
      { "GOTO"    , typecheck_rnum }
   };
   static const int types_len = ARRLEN(types);

   node_t *n;
   const typehandler_t *type;

   (void) _;
   n = tree_dat(t);

   for (int i = 0; i < types_len; i++) {
      type = types + i;
      if (strcmp(n->tag, type->kind))
         continue;
      (*type->check)(n);
      return;
   }
}

static void typecheck_adj(node_t *n) {
   ret = query(QUERYKIND_ADJ, n->run, &val);
   if (!ret) {
      reason = msgs.err.sem.bad_adj;
      enode = n;
      semerr();
   }
}

static bool validate_name(const char *s, int *ret) {
   const tree_t *dp, *ch;
   const node_t *n;
   int i, clen;

   dp = tree_child(pt, 1);
   clen = tree_clen(dp);

   for (i = 0; i < clen; i++) {
      ch = tree_child(dp, i);
      n = tree_dat(ch);
      if (!strcmp(n->run, s)) {
         *ret = i;
         return true;
      }
   }
   return false;
}

static bool validate_pronoun(const char *s) {
   static const char *prns[] = {
      /* pronoun */
      "me", "you", "thee",
      /* reflexive */
      "myself", "yourself", "thyself"
   };
   static const int prns_len = ARRLEN(prns);

   const char *prn;

   for (int i = 0; i < prns_len; i++) {
      prn = prns[i];
      if (!strcmp(prn, s))
         return true;
   }
   return false;
}

static void mark_whom(char *tag, int num) {
   /*
    * node.tag = 31 chars + \0
    * but 26 chars available for `num`
    * because "CHAR-" is 5 chars long.
    *
    * Given that `num` is int and >= 0,
    * buf[10] would suffice since
    * max_int = 2,147,483,647 = 10 chars
    */
   #define NAME_BUFSIZ 11

   static char buf[NAME_BUFSIZ];  /* for - */

   snprintf(buf, NAME_BUFSIZ, "-%d", num);
   strcat(tag, buf);
}

static void typecheck_noun(node_t *n) {
   const char *nounkind;

   ret = validate_name(n->run, &val);
   if (ret) {
      mark_whom(n->tag, val);
      return;
   }

   ret = validate_pronoun(n->run);
   if (ret) return;

   ret = query(QUERYKIND_NOUN, n->run, &val);
   if (!ret) {
      reason = msgs.err.sem.bad_noun;
      enode = n;
      semerr();
   }

   nounkind = val ? "-POS" : "-NEG";
   strcat(n->tag, nounkind);
}

static void typecheck_char(node_t *n) {
   ret = validate_name(n->run, &val);
   if (!ret) {
      reason = msgs.err.sem.bad_name;
      enode = n;
      semerr();
   }
   mark_whom(n->tag, val);
}

static void typecheck_comp(node_t *n) {
   const char *compkind;

   ret = query(QUERYKIND_COMP, n->run, &val);
   if (!ret) {
      reason = msgs.err.sem.bad_comp;
      enode = n;
      semerr();
   }

   compkind = val ? "-POS" : "-NEG";
   strcat(n->tag, compkind);
}

static bool is_valid_rnum(const char *rnum) {
   #define NUMS_LEN 9

   typedef const char *nums_t[NUMS_LEN];

   static nums_t hundreds = {
      "CM", "DCCC", "DCC", "DC", "D", "CD", "CCC", "CC", "C"
   };
   static nums_t tens = {
      "XC", "LXXX", "LXX", "LX", "L", "XL", "XXX", "XX", "X"
   };
   static nums_t units = {
      "IX", "VIII", "VII", "VI", "V", "IV", "III", "II", "I"
   };
   static nums_t *arr[] = { &hundreds, &tens, &units };
   static const int arr_len = ARRLEN(arr);

   int i, k, nlen;

   for (i = 0; i < arr_len; i++) {
      for (k = 0; k < NUMS_LEN; k++) {
         nlen = strlen((*arr[i])[k]);
         if (!strncmp(rnum, (*arr[i])[k], nlen))
            break;
      }
      if (k < NUMS_LEN)
         rnum += nlen;
   }

   return strlen(rnum) == 0 ? true : false;
}

static void typecheck_rnum(node_t *n) {
   ret = is_valid_rnum(n->run);
   if (!ret) {
      reason = msgs.err.sem.bad_rnum;
      enode = n;
      semerr();
   }
}

static inline void semerr(void) {
   err_template(tell, Cbred, "<semantic error> ");
}

static void tell(void) {
   int lnum, lpos;
   line_t *l;

   lnum = enode->lnum;
   lpos = enode->lpos;
   l = arr_peek(ls, lnum - 1);

   ffmtwrt(stderr,
      "%s " Cbcyan "%s" Creset "\n"
      "[%s:%d:%d] " Cbwhite "note:" Creset " problematic at here\n"
      "%4d|%.*s" Cbblue "%s" Creset "%s\n",
      reason, enode->run,
      sfname, lnum, lpos,
      lnum, lpos - 1, l->run, enode->run, &l->run[lpos - 1 + enode->len - 1]
   );
}
