#include "transpile.h"
#include "transpile.type.h"

static FILE *temp_main;
static FILE *temp_act;
static arr_t *actfiles;
static node_t *n;
static int clen;
static int speaker;

extern void transpile(optflg_t *of, optval_t *ov, tree_t *pt) {
   actfiles = arr_create();
   transpile2C(pt);
}

static void transpile2C(tree_t *pt) {
   temp_act = NULL;
   temp_main = sfopen(TEMPFILE_MAIN_NAME, "w");
   // main에 인클루드파일 같은거 하는함수
   tree_pre_traverse(pt, generate_c_router, 0);
   sfputs(temp_act, "}");
   // act 함수정의가 적힌 파일을 include 다음에 붙여넣기
   // 본문은 본문파일에 따로 적고
   // 그 뒤에 메인함수 작성
}

static void generate_c_router(tree_t *t, int _) {
   typedef struct typehandler {
      nodekind_t kind;
      generator_t *gen;
   } typehandler_t;

   static const typehandler_t types[] = {
      { "TITLE", gen_title },
      { "D.P.", gen_dp },
      { "ACT", gen_act },
      { "SCENE", gen_scene },
      { "ENTER", gen_enter },
      { "EXIT", gen_exit },
      { "EXEUNT", gen_exeunt },
      { "LINE", gen_line },
      { "ASSIGN", gen_asgn },
      { "OUT-NUM", gen_outn },
      { "OUT-CHAR", gen_outc },
      { "IN-NUM", gen_inn },
      { "IN-CHAR", gen_inc },
      { "GOTO", gen_goto },
      { "COND", gen_cond }, // 전역변수 bool cond_xx = ...;
      { "IF", gen_if },
      { "PUSH", gen_push },
      { "POP", gen_pop }
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
      (*type->gen)(t);
      return;
   }
}

static void resolve_const(tree_t *t) {
   // resolve_const 함수 만들어서 재귀적으로 처리
   // 값이나 연산자 결과를 괄호로 감싸기
   // for (int i = 0; i < clen - 2; i++)
   //    ffmtwrt(temp_act, " * 1");
}

static void gen_title(tree_t *t) {
   n = tree_dat(t);
   ffmtwrt(temp_main,
      "/* %s */\n",
      n->run
   );
}

static void gen_dp(tree_t *t) {
   n = tree_dat(t);
   clen = tree_clen(t);

   for (int i = 0; i < clen; i++) {
      ffmtwrt(temp_main,
         "static bool %s = false;\n",
         TREE_CHDAT(t, i)->run
      );
   }
}

static void gen_act(tree_t *t) {
   node_t *n;
   char *buf;
   int buflen;

   if (temp_act)
      sfputs(temp_act, "}");

   n = tree_dat(t);
   buflen = strlen(TEMPFILE_ACT_NAME) + n->len;
   buf = smalloc(buflen);  /* n.len has a room for \0 */
   strcpy(buf, TEMPFILE_ACT_NAME);
   strcat(buf, n->run);
   arr_append(actfiles, buf, buflen);
   temp_act = sfopen(buf, "w");

   ffmtwrt(temp_act, "void spl_act_%s(void) {\n", n->run);
   ffmtwrt(temp_main, "   spl_act_%s();\n", n->run);
}

static void gen_scene(tree_t *t) {
   n = tree_dat(t);
   ffmtwrt(temp_act, "   spl_scene_%s:;\n", n->run);
}

static void gen_enter(tree_t *t) {
   clen = tree_clen(t);
   for (int i = 0; i < clen; i++)
      ffmtwrt(temp_act,
         "   spl_enter(%s);\n",
         TREE_CHDAT(t, i)
      );
}

static void gen_exit(tree_t *t) {
   ffmtwrt(temp_act,
      "   spl_exit(%s);\n",
      TREE_CHDAT(t, 0)
   );
}

static void gen_exeunt(tree_t *t) {
   clen = tree_clen(t);
   if (!clen)
      ffmtwrt(temp_act,
         "   spl_exeunt();\n"
      );
   else
      for (int i = 0; i < clen; i++)
         ffmtwrt(temp_act,
            "   spl_exit(%s);\n",
            TREE_CHDAT(t, i)
         );
}

static void gen_line(tree_t *t) {
   n = tree_dat(t);
   sscanf(n->run, "CHAR-%d", &speaker);
}

static void gen_asgn(tree_t *t) {
   clen = tree_clen(t);
   ffmtwrt(temp_act,
      "   %s = %s",
      TREE_CHDAT(t, clen - 1),
      TREE_CHDAT(t, clen - 2)
   );
   resolve_const(t);
   ffmtwrt(temp_act, ";\n");
}

static void gen_outn(tree_t *t) {
   ffmtwrt(temp_act,
      "   printf(\"\%d\", CHAR-%d);\n",
      speaker
   );
}

static void gen_outc(tree_t *t) {
   ffmtwrt(temp_act,
      "   printf(\"\%c\", CHAR-%d);\n",
      speaker
   );
}

static void gen_inn(tree_t *t) {
   ffmtwrt(temp_act,
      "   scanf\"\%d\", &CHAR-%d);\n",
      speaker
   );
}

static void gen_inc(tree_t *t) {
   ffmtwrt(temp_act,
      "   scanf\"\%c\", &CHAR-%d);\n",
      speaker
   );
}

static void gen_goto(tree_t *t) {
   n = tree_dat(t);
   ffmtwrt(temp_act,
      "   goto spl_scene_%s;\n",
      n->run
   );
}

static void gen_cond(tree_t *t) {
   char neg_ch, ineq_ch;
   tree_t *lhs, *rhs;
   node_t *neg, *ineq;

   lhs = tree_child(t, 0);
   rhs = tree_child(t, 1);
   neg = TREE_CHDAT(t, 2);
   ineq = TREE_CHDAT(t, 3);

   neg_ch = !strcmp(neg->run, "true") ? '!' : ' ';
   ineq_ch = !strcmp(ineq->run, "COMPARE_POS") ? '>' : '<';

   ffmtwrt(temp_act, "   cond = %c(", neg_ch);
   resolve_const(lhs);
   ffmtwrt(temp_act, " %c ", ineq_ch);
   resolve_const(rhs);
   ffmtwrt(temp_act, ");\n");
}

static void gen_if(tree_t *t) {
   char neg_ch;
   node_t *neg;

   neg = TREE_CHDAT(t, 0);
   neg_ch = !strcmp(neg->run, "true") ? '!' : ' ';

   ffmtwrt(temp_act,
      "   if (%ccond)\n",
      neg_ch
   );
   // TODO: treat sub-statement
}

static void gen_push(tree_t *t) {
   // ffmtwrt(temp_act,
   //    "   spl_push(CHAR-%d,
}

static void gen_pop(tree_t *t) {

}
