#include "typecheck.h"
#include "typecheck.type.h"

extern tree_t *pt;
static const node_t *enode;

extern void typecheck(optflg_t *of, optval_t *ov) {
   // Preprocess
   coalesce_title();
   tree_post_traverse(pt, coalesce_name, 0);

   // Type-check
   tree_post_traverse(pt, typecheck_router, 0);
}

static void coalesce_title(void) {
   const tree_t *title;
   node_t *n;
   char *buf;
   int i, clen, rlen;

   // Get the title node
   title = tree_child(pt, 0);
   n = tree_dat(title);
   clen = tree_clen(title);

   // Calculate the length of the title
   rlen = 0;
   for (i = 0; i < clen; i++)
      // notice that len == strlen(run) + 1
      rlen += TREE_CHDAT(title, i)->dat.s.len;

   rlen++;
   buf = smalloc(rlen);
   buf[0] = '\0';
   for (i = 0; i < clen; i++) {
      strcat(buf, TREE_CHDAT(title, i)->dat.s.run);
      strcat(buf, " ");
   }
   buf[rlen - 2] = '\0';
   setnds(n, buf, rlen);
}

static void coalesce_name(tree_t *t, int _) {
   node_t *n;
   char *buf;
   int i, clen, rlen;

   (void) _;
   // Check if this is a character node
   n = tree_dat(t);
   if (n->kind != NODEKIND_CHDECL)
      return;

   // Calculate the length of the name
   clen = tree_clen(t);
   rlen = 0;
   for (i = 0; i < clen; i++)
      // notice that len == strlen(run) + 1
      rlen += TREE_CHDAT(t, i)->dat.s.len;

   /*
    * input  = ["the", "misty", "Ghost"]
    * output = "the_misty_Ghost00"
    *    where 0 = \0
    */
   rlen++;
   buf = smalloc(rlen);
   buf[0] = '\0';
   for (i = 0; i < clen; i++) {
      strcat(buf, TREE_CHDAT(t, i)->dat.s.run);
      strcat(buf, "_");
   }
   buf[rlen - 2] = '\0';
   setnds(n, buf, rlen);
}

static void typecheck_router(tree_t *t, int _) {
   typedef struct typehandler {
      nodekind_t kind;
      typechecker_t *check;
   } typehandler_t;

   static const typehandler_t types[] = {
      { NODEKIND_ADJ    , typecheck_adj  },
      { NODEKIND_NOUN   , typecheck_noun },
      { NODEKIND_ASSIGN , typecheck_adj  },
      { NODEKIND_EQ     , typecheck_adj  },
      { NODEKIND_INEQ   , typecheck_comp },
      { NODEKIND_ROMNUM , typecheck_rnum },
   };
   static const int types_len = ARRLEN(types);

   node_t *n;
   const typehandler_t *type;

   (void) _;
   n = tree_dat(t);

   for (int i = 0; i < types_len; i++) {
      type = types + i;
      if (n->kind != type->kind)
         continue;
      (*type->check)(n);
      return;
   }
}

static void typecheck_adj(node_t *n) {
   bool ret;
   int val;

   ret = query(QUERYKIND_ADJ, n->dat.s.run, &val);
   if (!ret) {
      reason = msgs.err.sem.bad_adj;
      enode = n;
      semerr();
   }
}

static int is_pronoun(const char *s) {
   static const char *prns[] = {
   /*  objective       reflexive  */
      KEYWRD_ME    , KEYWRD_MYSELF,
      KEYWRD_YOU_L , KEYWRD_YRSELF,
      KEYWRD_THEE  , KEYWRD_TYSELF,
   };
   static const int prns_len = ARRLEN(prns);

   const char *prn;

   for (int i = 0; i < prns_len; i++) {
      prn = prns[i];
      if (!strcmp(prn, s))
         return i;
   }
   return -1;
}

static void typecheck_noun(node_t *n) {
   bool ret;
   int ptype, val;
   nodekind_t kind;

   ptype = is_pronoun(n->dat.s.run);
   switch (ptype) {
      case 0 : /* fall-through */
      case 1 : kind = NODEKIND_P1; break;
      case 2 : /* fall-through */
      case 3 : /* fall-through */
      case 4 : /* fall-through */
      case 5 : kind = NODEKIND_P2; break;
      default : goto handle_noun;  /* not a pronoun */
   }
   n->kind = kind;
   return;

   handle_noun:
   ret = query(QUERYKIND_NOUN, n->dat.s.run, &val);
   if (!ret) {
      reason = msgs.err.sem.bad_noun;
      enode = n;
      semerr();
   }

   kind = val ? NODEKIND_PNOUN : NODEKIND_NNOUN;
   n->kind = kind;
}

static void typecheck_comp(node_t *n) {
   bool ret;
   int val;
   nodekind_t kind;

   ret = query(QUERYKIND_COMP, n->dat.s.run, &val);
   if (!ret) {
      reason = msgs.err.sem.bad_comp;
      enode = n;
      semerr();
   }

   kind = val ? NODEKIND_GT : NODEKIND_LT;
   n->kind = kind;
}

static bool is_rnum(const char *rnum) {
   #define PLACE_LEN 9
   typedef const char *place_t[PLACE_LEN];

   static place_t ps[] = {
      { "CM", "DCCC", "DCC", "DC", "D", "CD", "CCC", "CC", "C" }, /* 100 */
      { "XC", "LXXX", "LXX", "LX", "L", "XL", "XXX", "XX", "X" }, /*  10 */
      { "IX", "VIII", "VII", "VI", "V", "IV", "III", "II", "I" }  /*   1 */
   };
   static const int ps_len = ARRLEN(ps);

   int i, k, nlen;
   place_t *p;
   const char *dat;

   for (i = 0; i < ps_len; i++) {
      p = ps + i;
      for (k = 0; k < PLACE_LEN; k++) {
         dat = (*p)[k];
         nlen = strlen(dat);
         if (!strncmp(rnum, dat, nlen))
            goto increment;
      }
      continue;  /* k == PLACE_LEN */
      increment: rnum += nlen;
   }

   return strlen(rnum) == 0 ? true : false;
}

static void typecheck_rnum(node_t *n) {
   bool ret;

   ret = is_rnum(n->dat.s.run);
   if (!ret) {
      reason = msgs.err.sem.bad_rnum;
      enode = n;
      semerr();
   }
}

static inline void semerr(void) {
   err_template(tell, Cbred, "\n<semantic error> ");
}

static void tell(void) {
   int lnum, lpos;
   line_t *l;

   lnum = enode->lnum;
   lpos = enode->lpos;
   l = arr_peek(ls, lnum - 1);

   fmtwrt(
      "%s " Cbcyan "%s" Creset "\n"
      "[%s:%d:%d] " Cbwhite "note:" Creset " problematic at here\n"
      "%4d|%.*s" Cbblue "%s" Creset "%s\n",
      reason, enode->dat.s.run,
      sfname, lnum, lpos,
      lnum, lpos - 1, l->run,
      enode->dat.s.run, &l->run[lpos - 1 + enode->dat.s.len - 1]
   );
}
