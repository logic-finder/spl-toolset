#include "typecheck.h"
#include "typecheck.internals.h"

// TODO: typecheck.c의 필요성이 없는 것 같은데 parse.c로 넘어간 기능은 없애고
// 남은 기능은 ctxcheck.c로 옮기도록 하자

extern void typecheck(compile_ctx_t *cctx) {
   typecheck_ctx_t tctx;

   tctx.pt = cctx->pt;
   tctx.title = tree_child(tctx.pt, 0);
   tctx.dp = tree_child(tctx.pt, 1);
   tctx.ov = cctx->ov;
   tctx.ls = cctx->ls;

   /* Preprocesses */
   coalesce_title(&tctx);
   coalesce_name(&tctx);
   check_namecol(&tctx);

   /* Type-checks */
   tree_post_traverse(tctx.pt, typecheck_router, 0, &tctx);
}

static inline void coalesce_title(typecheck_ctx_t *tctx) {
   coalesce_childstr(tctx->title);
}

static void coalesce_name(typecheck_ctx_t *tctx) {
   tree_t *chardecl;
   const size_t dp_len = tree_clen(tctx->dp);

   for (size_t i = 0; i < dp_len; i++) {
      chardecl = tree_child(tctx->dp, i);
      coalesce_childstr(chardecl);
   }
}

static void check_namecol(typecheck_ctx_t *tctx) {
   const char *curr, *prev;
   node_t *cn, *pn;
   size_t clen;

   clen = tree_clen(tctx->dp);
   for (size_t i = 1; i < clen; i++) {
      cn = tree_chdat(tctx->dp, i);
      curr = cn->dat.s.run;
      for (size_t k = 0; k < i; k++) {
         pn = tree_chdat(tctx->dp, k);
         prev = pn->dat.s.run;
         if (!strcmp(curr, prev))
            semerr_dupname(tctx, cn, pn);
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
      // { NODEKIND_ADJ    , typecheck_adj  },
      // { NODEKIND_NOUN   , typecheck_noun },
      // { NODEKIND_ASGN2  , typecheck_adj  },
      { NODEKIND_EQ     , typecheck_adj  },
      { NODEKIND_INEQ   , typecheck_comp },
      { NODEKIND_GT     , typecheck_adj  },
      { NODEKIND_LT     , typecheck_adj  },
      { NODEKIND_ROMNUM , typecheck_rnum }
   };
   static const size_t types_len = ARRLEN(types);

   typecheck_ctx_t *tctx = ctx;
   const typehandler_t *type;

   (void) lv;
   tctx->t = t;
   tctx->n = tree_dat(tctx->t);

   for (size_t i = 0; i < types_len; i++) {
      type = types + i;
      if (tctx->n->kind != type->kind)
         continue;
      (*type->check)(tctx);
      return;
   }
}

static void typecheck_name(typecheck_ctx_t *tctx) {
   if (query_name(tctx->n->dat.s.run))
      return;
   reason = msgs.err.sem.bad_name;
   semerr_badword(tctx);
}

static void typecheck_adj(typecheck_ctx_t *tctx) {
   if (query_adj(tctx->n->dat.s.run))
      return;
   reason = msgs.err.sem.bad_adj;
   semerr_badword(tctx);
}

static void typecheck_noun(typecheck_ctx_t *tctx) {
   bool ret;
   int ptype, val;
   nodekind_t kind;

   ptype = is_pronoun(tctx->n->dat.s.run);
   switch (ptype) {
      case 0 : /* fall-through */
      case 1 : kind = NODEKIND_P1; break;
      case 2 : /* fall-through */
      case 3 : /* fall-through */
      case 4 : /* fall-through */
      case 5 : kind = NODEKIND_P2; break;
      default : goto handle_noun;  /* not a pronoun */
   }
   tctx->n->kind = kind;
   return;

handle_noun:
   ret = query_noun(tctx->n->dat.s.run, &val);
   if (!ret) {
      reason = msgs.err.sem.bad_noun;
      semerr_badword(tctx);
   }

   kind = val ? NODEKIND_PNOUN : NODEKIND_NNOUN;
   tctx->n->kind = kind;
}

static void typecheck_comp(typecheck_ctx_t *tctx) {
   bool ret;
   int val;
   nodekind_t kind;

   ret = query_comp(tctx->n->dat.s.run, &val);
   if (!ret) {
      reason = msgs.err.sem.bad_comp;
      semerr_badword(tctx);
   }

   kind = val ? NODEKIND_GT : NODEKIND_LT;
   tctx->n->kind = kind;
}

static void typecheck_rnum(typecheck_ctx_t *tctx) {
   if (is_rnum(tctx->n->dat.s.run))
      return;
   reason = msgs.err.sem.bad_rnum;
   semerr_badword(tctx);
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

static void setnds(node_t *n, char *s, int l) {
   n->datkind = DATKIND_STR;
   n->dat.s.run = s;
   n->dat.s.len = l;
}

static inline void print_errheader(void) {
   safe_fputs(stdout, Cbred "\n<semantic error> " Creset);
}

static void semerr_badword(typecheck_ctx_t *tctx) {
   size_t lnum, lpos;
   line_t *l;

   lnum = tctx->n->lnum;
   lpos = tctx->n->lpos;
   l = array_peek(tctx->ls, lnum - 1);

   print_errheader();
   safe_vprintf(
      "%s " Cbcyan "%s" Creset "\n"
      "[%s:%zu:%zu] " Cbwhite "note:" Creset " problematic at here\n"
      "%4d|%.*s" Cbblue "%s" Creset "%s\n",
      reason, tctx->n->dat.s.run,
      tctx->ov->src, lnum, lpos,
      lnum, lpos - 1, l->run,
      tctx->n->dat.s.run, &l->run[lpos - 1 + tctx->n->dat.s.len]
   );
   exit(EXIT_FAILURE);
}

static void semerr_dupname(
   typecheck_ctx_t *tctx,
   node_t * restrict curr,
   node_t * restrict prev
) {
   line_t *cl, *pl;

   cl = array_peek(tctx->ls, curr->lnum - 1);
   pl = array_peek(tctx->ls, prev->lnum - 1);

   print_errheader();
   safe_vprintf(
      "duplicate name " Cbcyan "%s" Creset "\n"
      "[%s:%zu:%zu] " Cbwhite "note:" Creset " at here\n"
      "%4d|%.*s" Cbblue "%s" Creset "%s"
      "[%s:%zu:%zu] " Cbwhite "note:" Creset " first declared at here\n"
      "%4d|%.*s" Cbblue "%s" Creset "%s",
      curr->dat.s.run,
      tctx->ov->src, curr->lnum, curr->lpos,
      curr->lnum, curr->lpos - 1, cl->run,
         curr->dat.s.run, &cl->run[curr->lpos - 1 + curr->dat.s.len],
      tctx->ov->src, prev->lnum, prev->lpos,
      prev->lnum, prev->lpos - 1, pl->run,
         prev->dat.s.run, &pl->run[prev->lpos - 1 + prev->dat.s.len]
   );
   exit(EXIT_FAILURE);
}

static void coalesce_childstr(tree_t *t) {
   node_t *dat, *chdat;
   char *buf;
   size_t clen, bufsiz;

   clen = tree_clen(t);
   bufsiz = 0;

   for (size_t i = 0; i < clen; i++) {
      chdat = tree_chdat(t, i);
      bufsiz += chdat->dat.s.len + 1;  /* +1 for ' ' or \0 */
   }
   bufsiz--;  /* -1 not to count \0 */

   buf = safe_malloc(bufsiz + 1);  /* +1 for \0 */

   buf[0] = '\0';
   for (size_t i = 0; i < clen; i++) {
      chdat = tree_chdat(t, i);
      strcat(buf, chdat->dat.s.run);
      strcat(buf, " ");
   }
   buf[bufsiz] = '\0';  /* overwrite the last ' ' */

   dat = tree_dat(t);
   setnds(dat, buf, bufsiz);
}
