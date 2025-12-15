#include "ctxcheck.h"
#include "ctxcheck.type.h"

static trace_t *traces;
static int traces_len;
static int cnt_onstage;
static int speaker, listener;
static tree_t *nrtv, *act;
static node_t *enode;

extern void ctxcheck(optflg_t *of, optval_t *ov) {
   // Initialize global variables
   nrtv = tree_child(pt, 2);
   cnt_onstage = 0;

   // Context-check
   init_traces();
   tree_pre_traverse(nrtv, ctxcheck_router, 0);
}

static void init_traces(void) {
   trace_t *trace;
   tree_t *dp;
   int dp_len;

   dp = tree_child(pt, 1);
   dp_len = traces_len = tree_clen(dp);
   traces = smalloc(dp_len * ESIZ(traces));

   for (int idx = 0; idx < dp_len; idx++) {
      trace = traces + idx;
      trace->onstage = false;
      trace->loc = NULL;
   }
}

static void ctxcheck_router(tree_t *t, int _) {
   node_t *n;

   (void) _;
   n = tree_dat(t);

   switch (n->kind) {
      case NODEKIND_ACT    : ctxcheck_act   (t); break;
      case NODEKIND_SCENE  : ctxcheck_scene (t); break;
      case NODEKIND_ENTER  : ctxcheck_enter (t); break;
      case NODEKIND_EXIT   : ctxcheck_exit  (t); break;
      case NODEKIND_EXEUNT : ctxcheck_exeunt(t); break;
      case NODEKIND_LINE   : ctxcheck_line  (t); break;
      case NODEKIND_ASSIGN : ctxcheck_asgn  (t); break;
      case NODEKIND_GOTO   : ctxcheck_goto  (t); break;
      case NODEKIND_COND   : ctxcheck_cond  (t); break;
      case NODEKIND_PUSH   : ctxcheck_push  (t); break;
      case NODEKIND_POP    : ctxcheck_pop   (t); break;
      case NODEKIND_P1     : ctxcheck_p1    (t); break;
      case NODEKIND_P2     : ctxcheck_p2    (t); break;
      default : ;
   }
}

static void ctxcheck_act(tree_t *t) {
   act = t;
}

static void ctxcheck_enter(tree_t *t) {
   tree_t *dp;
   node_t *n;
   int clen, idx;

   clen = tree_clen(t);
   for (int i = 0; i < clen; i++) {
      n = tree_chdat(t, i);
      idx = n->dat.n;
      if (isonstage(idx)) {
         reason = msgs.err.sem.already_onstage;
         enode = n;
         dp = tree_child(pt, 1);
         semwarn_stage(TREE_CHDAT(dp, idx)->dat.s.run);
      }
      traces[idx].onstage = true;
      traces[idx].loc = n;
   }
   cnt_onstage += clen;
}

static void ctxcheck_exit(tree_t *t) {
   tree_t *dp;
   node_t *n;
   int idx;

   n = tree_chdat(t, 0);
   idx = n->dat.n;
   if (!isonstage(idx)) {
      reason = msgs.err.sem.already_offstage;
      enode = n;
      dp = tree_child(pt, 1);
      semwarn_stage(TREE_CHDAT(dp, idx)->dat.s.run);
   }
   traces[idx].onstage = false;
   traces[idx].loc = n;
   cnt_onstage--;
}

static void ctxcheck_exeunt(tree_t *t) {
   tree_t *dp;
   node_t *n;
   int clen, idx;

   n = tree_dat(t);
   clen = tree_clen(t);
   if (!clen)
      goto exit_all;
   else
      goto exit_some;

   exit_all:
      if (!cnt_onstage) {
         reason = msgs.err.sem.emptystage;
         enode = n;
         semerr_exeunt();
      }
      for (int i = 0; i < traces_len; i++) {
         if (!isonstage(i))
            continue;
         traces[i].onstage = false;
         traces[i].loc = n;
      }
      cnt_onstage = 0;
   return;

   exit_some:
      for (int i = 0; i < clen; i++) {
         n = tree_chdat(t, i);
         idx = n->dat.n;
         if (!isonstage(idx)) {
            reason = msgs.err.sem.already_offstage;
            enode = n;
            dp = tree_child(pt, 1);
            semwarn_stage(TREE_CHDAT(dp, idx)->dat.s.run);
         }
         traces[idx].onstage = false;
         traces[idx].loc = n;
      }
      cnt_onstage -= clen;
   return;
}

static void ctxcheck_line(tree_t *t) {
   tree_t *dp, *character;
   node_t *n;
   int idx;

   character = tree_child(t, 0);
   n = tree_dat(character);
   idx = n->dat.n;
   if (!isonstage(idx)) {
      reason = msgs.err.sem.not_onstage;
      enode = n;
      dp = tree_child(pt, 1);
      semerr_line(TREE_CHDAT(dp, idx)->dat.s.run);
   }
   speaker = idx;
}

static void ctxcheck_asgn(tree_t *t) {
   enode = tree_dat(t);
   aretheretwo(
      msgs.err.sem.solitary,
      msgs.err.sem.vague_ref
   );
   listener = whoareyou(speaker);
   graft_tree(t, listener, NODEKIND_SUBJ);
}

static void ctxcheck_goto(tree_t *t) {
   const tree_t *root, *root_child;
   const char *romnum, *against;
   node_t *n;
   int mark, root_len;

   n = tree_dat(t);
   mark = n->dat.n;
   romnum = TREE_CHDAT(t, 0)->dat.s.run;

   if (mark == NODEKIND_ACT)
      root = nrtv;  /* root_child is NODEKIND_ACT   */
   else  /* Scene */
      root = act;   /* root_child is NODEKIND_SCENE */

   root_len = tree_clen(root);
   for (int i = 1; i < root_len; i++) {  /* [0] = NODEKIND_ROMNUM */
      root_child = tree_child(root, i);
      against = TREE_CHDAT(root_child, 0)->dat.s.run;
      if (!strcmp(romnum, against))
         return;  /* found */
   }

   if (root == nrtv)
      reason = msgs.err.sem.no_such_act;
   else
      reason = msgs.err.sem.no_such_scene;
   enode = n;
   semerr_goto(romnum);
}

static void ctxcheck_cond(tree_t *t) {
   node_t *lhs, *rhs_const_noun;
   tree_t *rhs, *rhs_const;
   int rhs_const_clen;

   lhs = TREE_CHDAT(t, 0);
   if (lhs->kind == NODEKIND_P2) {
      enode = lhs;
      goto cntcheck;
   }

   rhs = tree_child(t, 2);
   rhs_const = tree_child(rhs, 0);
   rhs_const_clen = tree_clen(rhs_const);
   rhs_const_noun = TREE_CHDAT(rhs_const, rhs_const_clen - 1);
   if (rhs_const_noun->kind == NODEKIND_P2) {
      enode = rhs_const_noun;
      goto cntcheck;
   }
   return;

   cntcheck: aretheretwo(
      msgs.err.sem.solitary,
      msgs.err.sem.vague_ref
   );
}

static void ctxcheck_stack(
   tree_t *t,
   const char * restrict solerr,
   const char * restrict referr
) {
   enode = tree_dat(t);
   aretheretwo(solerr, referr);
   listener = whoareyou(speaker);
   graft_tree(t, listener, NODEKIND_SUBJ);
}

static void ctxcheck_push(tree_t *t) {
   ctxcheck_stack(t, msgs.err.sem.solpush, msgs.err.sem.mulpush);
}

static void ctxcheck_pop(tree_t *t) {
   ctxcheck_stack(t, msgs.err.sem.solpop, msgs.err.sem.mulpop);
}

static void ctxcheck_p(tree_t *t, int p) {
   node_t *n;

   n = tree_dat(t);
   n->kind = NODEKIND_CHAR;
   setndn(n, p);
}

static void ctxcheck_p1(tree_t *t) {
   ctxcheck_p(t, speaker);
}

static void ctxcheck_p2(tree_t *t) {
   ctxcheck_p(t, listener);
}

static void ctxcheck_scene(tree_t *scene) {
   tree_t *stmt, *line;
   node_t *n;
   int scene_clen, line_clen;

   scene_clen = tree_clen(scene);
   for (int i = 0; i < scene_clen; i++) {
      stmt = tree_child(scene, i);
      n = tree_dat(stmt);
      if (n->kind != NODEKIND_LINE)
         continue;
      line = stmt;
      line_clen = tree_clen(line);
      for (int k = 0; k < line_clen; k++) {
         stmt = tree_child(line, k);
         n = tree_dat(stmt);
         if (n->kind != NODEKIND_GOTO)
            continue;
         if (k != line_clen - 1 || i != scene_clen - 1) {
            reason = msgs.err.sem.unreachable;
            enode = n;
            semerr_scene();
         }
      }
   }
}

static void aretheretwo(
   const char * restrict solerr,
   const char * restrict referr
) {
   switch (cnt_onstage) {
      case 2 : return;
      case 1  : reason = solerr; break;
      default : reason = referr;
   }
   semerr_onstagecnt();
}

static inline bool isonstage(int who) {
   return traces[who].onstage;
}

static int whoareyou(int me) {
   /* assumes there are only two characters on stage */
   for (int idx = 0; idx < traces_len; idx++) {
      if (idx == me)
         continue;
      if (isonstage(idx))
         return idx;
   }
   /* control never reaches here under the preposition */
   return -1;
}

static inline void header_err(void) {
   fputs(Cbred "<semantic error>" Creset " ", stderr);
}

static inline void header_warn(void) {
   fputs(Cbmagenta "<semantic warning>" Creset " ", stderr);
}

static void errtrace(void) {
   int lnum, lpos;
   line_t *l;

   lnum = enode->lnum;
   lpos = enode->lpos;
   l = arr_peek(ls, lnum - 1);

   ffmtwrt(stderr,
      "\n[%s:%d:%d] " Cbwhite "note:" Creset " problematic from this line\n"
      "%4d|%s\n",
      sfname, lnum, lpos,
      lnum, l->run
   );
}

static void semerr_exeunt(void) {
   header_err();
   fputs(reason, stderr);
   errtrace();
   exit(EXIT_FAILURE);
}

static void semerr_line(const char *s) {
   header_err();
   ffmtwrt(stderr, reason, s);
   errtrace();
   exit(EXIT_FAILURE);
}

static void semerr_goto(const char *s) {
   header_err();
   ffmtwrt(stderr, reason, s);
   errtrace();
   exit(EXIT_FAILURE);
}

static void semerr_scene(void) {
   header_err();
   fputs(reason, stderr);
   errtrace();
   exit(EXIT_FAILURE);
}

static void semerr_onstagecnt(void) {
   header_err();
   ffmtwrt(stderr, reason, cnt_onstage);
   errtrace();
   exit(EXIT_FAILURE);
}

static void semwarn_stage(const char *s) {
   header_warn();
   ffmtwrt(stderr, reason, s);
   errtrace();
   exit(EXIT_FAILURE);
}

static void graft_tree(
   tree_t *base,
   int num,
   nodekind_t kind
) {
   node_t n;
   tree_t *sub;

   n.datkind = DATKIND_INT;
   n.dat.n = num;
   n.kind = kind;
   sub = tree_plant(&n, sizeof n);
   (void) tree_graft(base, sub);
}
