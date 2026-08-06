#include "typecheck.h"
#include "typecheck.internals.h"

extern void typecheck(optflg_t *of, optval_t *ov) {
   tree_t *dp;

   (void) of, (void) ov;

   /* Preprocess */
   coalesce_title();
   dp = tree_child(pt, 1);
   coalesce_name(dp);
   check_namecol(dp);

   /* Type-check */
   tree_post_traverse(pt, typecheck_router, 0, NULL);
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
   buf = safe_malloc(rlen);
   buf[0] = '\0';
   for (i = 0; i < clen; i++) {
      strcat(buf, TREE_CHDAT(title, i)->dat.s.run);
      strcat(buf, " ");
   }
   buf[rlen - 2] = '\0';
   setnds(n, buf, rlen);
}

static void coalesce_name(tree_t *dp) {
   tree_t *chardecl;
   char *buf;
   int dp_len, chardecl_len, total_len;

   dp_len = tree_clen(dp);
   for (int i = 0; i < dp_len; i++) {
      chardecl = tree_child(dp, i);
      chardecl_len = tree_clen(chardecl);
      total_len = 0;

      for (int k = 0; k < chardecl_len; k++)
         // note that s.len == strlen(s.run) + 1
         total_len += TREE_CHDAT(chardecl, k)->dat.s.len;

      // fixme: no need to +1?
      buf = safe_malloc(total_len + 1);  /* a room for ' ' */
      buf[0] = '\0';
      for (int k = 0; k < chardecl_len; k++) {
         strcat(buf, TREE_CHDAT(chardecl, k)->dat.s.run);
         strcat(buf, " ");
      }
      buf[total_len - 1] = '\0';
      setnds(tree_dat(chardecl), buf, total_len);
   }
}

static void check_namecol(tree_t *dp) {
   const char *curr, *prev;
   node_t *cn, *pn;
   int clen;

   clen = tree_clen(dp);
   for (int i = 1; i < clen; i++) {
      cn = tree_chdat(dp, i);
      curr = cn->dat.s.run;
      for (int k = 0; k < i; k++) {
         pn = tree_chdat(dp, k);
         prev = pn->dat.s.run;
         if (!strcmp(curr, prev))
            semerr_dupname(cn, pn);
      }
   }
}

static void typecheck_router(tree_t *t, int lv, void *ctx) {
   typedef struct typehandler {
      nodekind_t kind;
      typechecker_t *check;
   } typehandler_t;

   static const typehandler_t types[] = {
      { NODEKIND_CHDECL , typecheck_name },
      { NODEKIND_ADJ    , typecheck_adj  },
      { NODEKIND_NOUN   , typecheck_noun },
      { NODEKIND_ASGN1  , typecheck_adj  },
      { NODEKIND_EQ     , typecheck_adj  },
      { NODEKIND_INEQ   , typecheck_comp },
      { NODEKIND_GT     , typecheck_adj  },
      { NODEKIND_LT     , typecheck_adj  },
      { NODEKIND_ROMNUM , typecheck_rnum }
   };
   static const int types_len = ARRLEN(types);

   node_t *n;
   const typehandler_t *type;

   (void) lv, (void) ctx;
   n = tree_dat(t);

   for (int i = 0; i < types_len; i++) {
      type = types + i;
      if (n->kind != type->kind)
         continue;
      (*type->check)(n);
      return;
   }
}

static void typecheck_name(node_t *n) {
   if (query_name(n->dat.s.run))
      return;
   reason = msgs.err.sem.bad_name;
   semerr_badword(n);
}

// TODO: summer's처럼 명사's는 형용사로 취급하기
// ㄴㄴ 형용사가 아니라 summer's day 자체가 하나의 명사
static void typecheck_adj(node_t *n) {
   if (query_adj(n->dat.s.run))
      return;
   reason = msgs.err.sem.bad_adj;
   semerr_badword(n);
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
   ret = query_noun(n->dat.s.run, &val);
   if (!ret) {
      reason = msgs.err.sem.bad_noun;
      semerr_badword(n);
   }

   kind = val ? NODEKIND_PNOUN : NODEKIND_NNOUN;
   n->kind = kind;
}

static void typecheck_comp(node_t *n) {
   bool ret;
   int val;
   nodekind_t kind;

   ret = query_comp(n->dat.s.run, &val);
   if (!ret) {
      reason = msgs.err.sem.bad_comp;
      semerr_badword(n);
   }

   kind = val ? NODEKIND_GT : NODEKIND_LT;
   n->kind = kind;
}

static bool is_rnum(const char *rnum) {
   size_t i, k, nlen;
   place_t *p;
   const char *dat;

   for (i = 0; i < ps_len; i++) {
      p = ps + i;
      for (k = 0; k < 9; k++) {
         dat = (*p)[k];
         nlen = strlen(dat);
         if (strncmp(rnum, dat, nlen))
            continue;
         rnum += nlen;
         break;
      }
   }

   return !strlen(rnum);
}

extern unsigned int interpret_romnum(const char *romnum) {
   place_t *p;
   const char *s;
   unsigned int v, temp, len;

   v = 0;
   for (size_t i = 0; i < ps_len; i++) {
      p = ps + i;
      for (size_t j = 0; j < 9; j++) {
         s = (*p)[j];
         len = strlen(s);
         if (strncmp(s, romnum, len))
            continue;
         temp = 9 - j;
         switch (i) {
            case 0: temp *= 100; break;
            case 1: temp *=  10; break;
            default: ;
         }
         v += temp;
         romnum += len;
         break;
      }
   }

   return v;
}

static void typecheck_rnum(node_t *n) {
   if (is_rnum(n->dat.s.run))
      return;
   reason = msgs.err.sem.bad_rnum;
   semerr_badword(n);
}

static inline void print_errheader(void) {
   safe_fputs(stdout, Cbred "\n<semantic error> " Creset);
}

static void semerr_badword(node_t *n) {
   int lnum, lpos;
   line_t *l;

   lnum = n->lnum;
   lpos = n->lpos;
   l = array_peek(ls, lnum - 1);

   print_errheader();
   safe_vprintf(
      "%s " Cbcyan "%s" Creset "\n"
      "[%s:%d:%d] " Cbwhite "note:" Creset " problematic at here\n"
      "%4d|%.*s" Cbblue "%s" Creset "%s\n",
      reason, n->dat.s.run,
      sfname, lnum, lpos,
      lnum, lpos - 1, l->run,
      n->dat.s.run, &l->run[lpos - 1 + n->dat.s.len - 1]
   );
   exit(EXIT_FAILURE);
}

static void semerr_dupname(
   node_t * restrict curr,
   node_t * restrict prev
) {
   line_t *cl, *pl;

   cl = array_peek(ls, curr->lnum - 1);
   pl = array_peek(ls, prev->lnum - 1);

   print_errheader();
   safe_vprintf(
      "duplicate name " Cbcyan "%s" Creset "\n"
      "[%s:%d:%d] " Cbwhite "note:" Creset " at here\n"
      "%4d|%.*s" Cbblue "%s" Creset "%s"
      "[%s:%d:%d] " Cbwhite "note:" Creset " first declared at here\n"
      "%4d|%.*s" Cbblue "%s" Creset "%s",
      curr->dat.s.run,
      sfname, curr->lnum, curr->lpos,
      curr->lnum, curr->lpos - 1, cl->run,
         curr->dat.s.run, &cl->run[curr->lpos - 1 + curr->dat.s.len - 1],
      sfname, prev->lnum, prev->lpos,
      prev->lnum, prev->lpos - 1, pl->run,
         prev->dat.s.run, &pl->run[prev->lpos - 1 + prev->dat.s.len - 1]
   );
   exit(EXIT_FAILURE);
}
