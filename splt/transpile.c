#include "transpile.h"
#include "transpile.internals.h"

extern void transpile(optflg_t *of, optval_t *ov) {
   tree_t *title, *dp, *nrtv;

   (void) of, (void) ov;
   fp = sfopen(FP_NAME, "w");
   title = tree_child(pt, 0);
   dp    = tree_child(pt, 1);
   nrtv  = tree_child(pt, 2);
   you_flag = false;

   // Code generation
   gen_header();
   gen_title(title);
   sfputs(fp, "int main(void) {\n");
   gen_locals(dp);
   tree_pre_traverse(nrtv, generate, 0, NULL);
   gen_cleanup();
   sfputs(fp, "\n" INDENT "return 0;\n}\n");

   sfclose(fp);
}

static void gen_header(void) {
   sfputs(fp, "#include \"splcore.h\"\n\n");
}

static void gen_title(tree_t *title) {
   node_t *n = tree_dat(title);
   ffmtwrt(fp, "/* %s */\n\n", n->dat.s.run);
}

static void gen_locals(tree_t *dp) {
   node_t *character;
   int clen;

   clen = tree_clen(dp);

   ffmtwrt(fp, INDENT "int dpsz = %d;\n", clen);
   sfputs(fp, INDENT "bool cond = false;\n");
   sfputs(fp, INDENT "stage_t *stage = stage_create(dpsz);\n");
   sfputs(fp, INDENT "persona_t *personae = init_personae(dpsz);\n");
   sfputs(fp, INDENT "memory_t *memories = init_memories(dpsz);\n\n");

   for (int idx = 0; idx < clen; idx++) {
      character = tree_chdat(dp, idx);
      ffmtwrt(fp,
         INDENT "stage_setname(stage, %d, \"%s\");\n",
         idx, character->dat.s.run
      );
   }
}

static void gen_cleanup(void) {
   sfputs(fp, "\n" INDENT "stage_destroy(stage);\n");
   sfputs(fp, INDENT "free(personae);\n");
   sfputs(fp, INDENT "cleanup_memories(memories, dpsz);\n");
}

static void generate(tree_t *t, int lv, void *ctx) {
   node_t *n;

   (void) lv, (void) ctx;
   n = tree_dat(t);
   switch (n->kind) {
      case NODEKIND_ACT    : gen_act   (t); break;
      case NODEKIND_SCENE  : gen_scene (t); break;
      case NODEKIND_ENTER  : gen_enter (t); break;
      case NODEKIND_EXIT   : gen_exit  (t); break;
      case NODEKIND_EXEUNT : gen_exeunt(t); break;
      case NODEKIND_LINE   : gen_line  (t); break;
      case NODEKIND_ASGN1  : /* fall-through */
      case NODEKIND_ASGN2  : gen_asgn  (t); break;
      case NODEKIND_OUT_N  : gen_outn  (t); break;
      case NODEKIND_OUT_C  : gen_outc  (t); break;
      case NODEKIND_IN_N   : gen_inn   (t); break;
      case NODEKIND_IN_C   : gen_inc   (t); break;
      case NODEKIND_GOTO   : gen_goto  (t); break;
      case NODEKIND_COND   : gen_cond  (t); break;
      case NODEKIND_IF     : gen_if    (t); break;
      case NODEKIND_PUSH   : gen_push  (t); break;
      case NODEKIND_POP    : gen_pop   (t); break;
      /* NODEKIND__DEPEND falls here; see `gen_if` */
      /* const and operators also falls here */
      default : return;
   }
}

static void eval_const(tree_t *cnst) {
   /*
    * CONST NODE STRUCTURE
    *    const -> [adj...] (noun | char)
    *    const -> op -> (const | const const)
    */
   tree_t *sub, *op, *noun;
   node_t *n;
   int clen, v;

   sub = op = tree_child(cnst, 0);
   n = tree_dat(sub);

   switch (n->kind) {
      case NODEKIND_SUM  : resolve_sum (op); return;
      case NODEKIND_DIFF : resolve_diff(op); return;
      case NODEKIND_PROD : resolve_prod(op); return;
      case NODEKIND_QUOT : resolve_quot(op); return;
      case NODEKIND_REM  : resolve_rem (op); return;
      case NODEKIND_SQRT : resolve_sqrt(op); return;
      case NODEKIND_SQUR : resolve_squr(op); return;
      case NODEKIND_CUBE : resolve_cube(op); return;
      case NODEKIND_2X   : resolve_2x  (op); return;
      case NODEKIND_FACT : resolve_fact(op); return;
      default : ;  /* not an operator */
   }

   clen = tree_clen(cnst);
   noun = tree_child(cnst, clen - 1);
   // fixme: here
   if (1) {
      sfputc(fp, '(');
      for (int i = 0; i < clen - 1; i++)
         sfputs(fp, "2 * ");
      resolve_noun(noun);
      sfputc(fp, ')');
   }
   else {
      v = 1;
      for (int i = 0; i < clen - 1; i++)
         v *= 2;
      ffmtwrt(fp, "%d * ", v);
      resolve_noun(noun);
   }
}

static void resolve_noun(tree_t *noun) {
   node_t *n;

   n = tree_dat(noun);
   switch (n->kind) {
      case NODEKIND_PNOUN : goto pnoun;
      case NODEKIND_NNOUN : goto nnoun;
      case NODEKIND_P1    : goto p1;
      case NODEKIND_P2    : goto p2;
      case NODEKIND_CHAR  : goto persona;
      /* control never reaches here */
      default : ;
   }

   pnoun:
      sfputc(fp, '1');
   return;

   nnoun:
      sfputs(fp, "-1");
   return;

   p1:
      ffmtwrt(fp, "personae[%d]", speaker);
   return;

   p2:
      ffmtwrt(fp, "personae[stage_whoareyou(stage, %d)]", speaker);
      you_flag = true;
   return;

   persona:
      ffmtwrt(fp, "personae[%d]", n->dat.n);
   return;
}

static void resolve_sqrt(tree_t *op) {
   resolve_unary(op, "op_sqrt(");
}

static void resolve_squr(tree_t *op) {
   resolve_unary(op, "op_squr(");
}

static void resolve_cube(tree_t *op) {
   resolve_unary(op, "op_cube(");
}

static void resolve_2x(tree_t *op) {
   resolve_unary(op, "(2 * (");
   sfputc(fp, ')');
}

static void resolve_fact(tree_t *op) {
   resolve_unary(op, "op_fact(");
}

static void resolve_sum(tree_t *op) {
   resolve_binary(op, '+');
}

static void resolve_diff(tree_t *op) {
   resolve_binary(op, '-');
}

static void resolve_prod(tree_t *op) {
   resolve_binary(op, '*');
}

static void resolve_quot(tree_t *op) {
   resolve_division(op, "quot");
}

static void resolve_rem(tree_t *op) {
   resolve_division(op, "rem");
}

static void resolve_division(tree_t *op, const char *s) {
   tree_t *cnst1, *cnst2;

   cnst1 = tree_child(op, 0);  /* lhs */
   cnst1 = tree_child(cnst1, 0);
   cnst2 = tree_child(op, 1);  /* rhs */
   cnst2 = tree_child(cnst2, 0);

   sfputs(fp, "div((");
   eval_const(cnst1);
   sfputs(fp, "), (");
   eval_const(cnst2);
   ffmtwrt(fp, ")).%s", s);
}

static void resolve_unary(tree_t *op, const char *s) {
   tree_t *cnst;

   cnst = tree_child(op, 0);
   sfputs(fp, s);
   eval_const(cnst);
   sfputc(fp, ')');
}

static void resolve_binary(tree_t *op, char c) {
   tree_t *cnst1, *cnst2;

   cnst1 = tree_child(op, 0);  /* lhs */
   cnst1 = tree_child(cnst1, 0);
   cnst2 = tree_child(op, 1);  /* rhs */
   cnst2 = tree_child(cnst2, 0);

   sfputc(fp, '(');
   eval_const(cnst1);
   ffmtwrt(fp, ") %c (", c);
   eval_const(cnst2);
   sfputc(fp, ')');
}

static void gen_act(tree_t *t) {
   node_t *n;

   n = tree_chdat(t, 0);  /* NODEKIND_ROMNUM */
   actnum = ((node_t *) tree_chdat(t, 0))->dat.s.run;
   ffmtwrt(fp,
      INDENT "\n%s_%s:;\n",
      KEYWRD_ACT,
      n->dat.s.run
   );
}

static void gen_scene(tree_t *t) {
   node_t *n;

   n = tree_chdat(t, 0);  /* NODEKIND_ROMNUM */
   ffmtwrt(fp,
      INDENT "\n%s_%s_%s_%s:;\n",
      KEYWRD_ACT,
      actnum,
      KEYWRD_SCENE,
      n->dat.s.run
   );
}

static void gen_enter(tree_t *t) {
   int clen, charidx;

   clen = tree_clen(t);
   for (int i = 0; i < clen; i++) {
      charidx = TREE_CHDAT(t, i)->dat.n;
      ffmtwrt(fp, INDENT "stage_enter(stage, %d);\n", charidx);
   }
}

static void gen_exit(tree_t *t) {
   int charidx;

   charidx = TREE_CHDAT(t, 0)->dat.n;
   ffmtwrt(fp, INDENT "stage_exit(stage, %d);\n", charidx);
}

static void gen_exeunt(tree_t *t) {
   int clen, charidx;

   clen = tree_clen(t);
   if (!clen)
      sfputs(fp, INDENT "stage_exeunt(stage);\n");
   else
      for (int i = 0; i < clen; i++) {
         charidx = TREE_CHDAT(t, i)->dat.n;
         ffmtwrt(fp, INDENT "stage_exit(stage, %d);\n", charidx);
      }
}

static void gen_line(tree_t *t) {
   speaker = TREE_CHDAT(t, 0)->dat.n;  /* NODEKIND_CHAR */
   ffmtwrt(fp,
      INDENT "assert_offstage(stage, %d);\n",
      speaker
   );
}

static void gen_asgn(tree_t *t) {
   ffmtwrt(fp, INDENT "assert_onlytwo(stage);\n");
   ffmtwrt(fp, INDENT "personae[stage_whoareyou(stage, %d)] = ", speaker);
   eval_const(tree_child(t, 0));
   sfputs(fp, ";\n");
}

static void gen_io(const char *kind) {
   ffmtwrt(fp, INDENT "assert_onlytwo(stage);\n");
   ffmtwrt(fp,
      INDENT "io_%s(personae, stage_whoareyou(stage, %d));\n",
      kind, speaker
   );
}

static void gen_outn(tree_t *_) {
   (void) _;
   gen_io("outn");
}

static void gen_outc(tree_t *_) {
   (void) _;
   gen_io("outc");
}

static void gen_inn(tree_t *_) {
   (void) _;
   gen_io("inn");
}

static void gen_inc(tree_t *_) {
   (void) _;
   gen_io("inc");
}

static void gen_goto(tree_t *t) {
   nodekind_t type;
   node_t *place;

   type = ((node_t *) tree_dat(t))->dat.n;
   place = tree_chdat(t, 0);

   if (type == NODEKIND_ACT)
      ffmtwrt(fp,
         INDENT "goto %s_%s;\n",
         KEYWRD_ACT, place->dat.s.run
      );
   else
      ffmtwrt(fp,
         INDENT "goto %s_%s_%s_%s;\n",
         KEYWRD_ACT, actnum,
         KEYWRD_SCENE, place->dat.s.run
      );
}

static void gen_cond(tree_t *t) {
   /*
    * COND NODE STRUCTURE
    *    [0] = lhs -> p1 | p2 | (p3 -> const)
    *    [1] = negate | affirm
    *    [2] = eq | ineq
    *    [3] = rhs -> const
    * RESULT EXAMPLE
    *    cond = !(lhs < rhs);
    *    cond = (lhs == rhs);
    */
   tree_t *lhs, *rhs, *p;
   node_t *mode, *comp;
   nodekind_t kind;

   lhs  = tree_child(t, 0);
   mode = tree_chdat(t, 1);
   comp = tree_chdat(t, 2);
   rhs  = tree_child(t, 3);
   p = tree_child(lhs, 0);
   kind = ((node_t *) tree_dat(p))->kind;

   if (kind == NODEKIND_P2)
      ffmtwrt(fp, INDENT "assert_onlytwo(stage);\n");

   sfputs(fp, INDENT "cond = ");
   if (mode->kind == NODEKIND_NEGATE)
      sfputc(fp, '!');
   sfputc(fp, '(');

   // Left-hand side
   switch (kind) {
      case NODEKIND_P1 :
         ffmtwrt(fp, "personae[%d]", speaker);
         break;
      case NODEKIND_P2 :
         ffmtwrt(fp, "personae[stage_whoareyou(stage, %d)]", speaker);
         break;
      case NODEKIND_P3 :
         eval_const(tree_child(p, 0));
         break;
      /* control never reaches here */
      default : ;
   }

   // Operator
   switch (comp->kind) {
      case NODEKIND_LT : ffmtwrt(fp, " < "); break;
      case NODEKIND_EQ : ffmtwrt(fp, " == "); break;
      case NODEKIND_GT : ffmtwrt(fp, " > "); break;
      /* control never reaches here */
      default : ;
   }

   // Right-hand side
   eval_const(tree_child(rhs, 0));
   sfputs(fp, ");\n");

   /*
    * Since the rhs constant may have "you" as a noun,
    * we need to put this assertion here also, not only
    * for the P2.
    */
   if (you_flag) {
      ffmtwrt(fp, INDENT "assert_onlytwo(stage);\n");
      you_flag = false;
   }
}

static void gen_if(tree_t *t) {
   node_t *mode, *stmtdat;
   tree_t *conseq, *stmt;

   mode = tree_chdat(t, 0);
   conseq = tree_child(t, 1);
   stmt = tree_child(conseq, 0);
   stmtdat = tree_dat(stmt);

   sfputs(fp, INDENT "if (");
   if (mode->kind == NODEKIND_NEGATE)
      sfputc(fp, '!');
   sfputs(fp, "cond) ");

   switch (stmtdat->kind) {
      case NODEKIND_ASGN1  : /* fall-through */
      case NODEKIND_ASGN2  : gen_asgn(stmt); break;
      case NODEKIND_OUT_N  : gen_outn(stmt); break;
      case NODEKIND_OUT_C  : gen_outc(stmt); break;
      case NODEKIND_IN_N   : gen_inn (stmt); break;
      case NODEKIND_IN_C   : gen_inc (stmt); break;
      case NODEKIND_GOTO   : gen_goto(stmt); break;
      case NODEKIND_COND   : gen_cond(stmt); break;
      case NODEKIND_PUSH   : gen_push(stmt); break;
      case NODEKIND_POP    : gen_pop (stmt); break;
      case NODEKIND_IF :
         sfputs(fp, "\n");
         gen_if(stmt);
         break;
      /* control never reaches here */
      default : ;
   }

   /*
    * Marks the node so as to prevent it from
    * getting a further process in `generate`.
    */
   stmtdat->kind = NODEKIND__DEPEND;
}

static void gen_push(tree_t *t) {
   tree_t *cnst;

   cnst = tree_child(t, 0);

   ffmtwrt(fp, INDENT "assert_onlytwo(stage);\n");
   ffmtwrt(fp,
      INDENT "stack_push(memories[stage_whoareyou(stage, %d)], ",
      speaker
   );
   eval_const(cnst);
   sfputs(fp, INDENT ");\n");
}

static void gen_pop(tree_t *_) {
   (void) _;
   ffmtwrt(fp, INDENT "assert_onlytwo(stage);\n");
   ffmtwrt(fp,
      INDENT "personae[stage_whoareyou(stage, %d)] = "
      "stack_pop(memories[stage_whoareyou(stage, %d)];\n",
      speaker, speaker
   );
}
