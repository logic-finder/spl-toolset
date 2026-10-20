#include "parse.c"
#include "test.h"

#define FIXTURE(fun) \
   do { \
      setup(&pctx); \
      if (!fun(&pctx)) fail++; \
      teardown(&pctx); \
   } while (0)

/* helpers */
static void setup(parse_ctx_t *pctx);
static void teardown(parse_ctx_t *pctx);
static void regtok(parse_ctx_t *pctx, const char *run);

/* testcases */
static bool type_a(parse_ctx_t *pctx);

int main(void) {
   parse_ctx_t pctx = {0};
   size_t fail = 0;

   test_title(__FILE__);
   dbload(&cfg);

   FIXTURE(type_a);

   dbunload();
   test_result(fail);

   return fail == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

static void setup(parse_ctx_t *pctx) {
   token_t dummy = {0};

   pctx->arena = arena_create();
   pctx->toks = array_create(NULL);
   pctx->pt = tree_plant(NULL, 0, pctx->arena);
   pctx->errs = array_create(NULL);
   pctx->idx = 0;

   graft_tree_n(pctx, pctx->pt, 0, NODEKIND_TITLE, &dummy);
   graft_tree_n(pctx, pctx->pt, 0, NODEKIND_DP, &dummy);
}

static void teardown(parse_ctx_t *pctx) {
   array_destroy(pctx->errs);
   array_destroy(pctx->toks);
   arena_destroy(pctx->arena);
}

static void regtok(parse_ctx_t *pctx, const char *run) {
   token_t tok = {0};

   tok.len = strlen(run);
   tok.run = arena_malloc(pctx->arena, tok.len + 1);  /* +1 for \0 */
   memcpy(tok.run, run, tok.len + 1);  /* \0 copied */
   array_append(pctx->toks, &tok, sizeof tok);
}

/* TYPE A: <adj|ap> (noun|np) */
static bool type_a(parse_ctx_t *pctx) {
   /* setup */
   tree_t *mock_stmt;
   const char *words[] = {
      "lying", "stupid", "coward"
   };

   mock_stmt = tree_plant(NULL, 0, pctx->arena);

   regtok(pctx, "You");
   regtok(pctx, words[0]);
   regtok(pctx, words[1]);
   regtok(pctx, words[2]);
   regtok(pctx, "!");

   pctx->len = array_size(pctx->toks);
   pctx->tok = array_peek(pctx->toks, 0);

   /* perform */
   parse_const(pctx, mock_stmt);

   /* validate */
   bool flag = true;
   node_t *n;
   size_t clen;
   tree_t *cnst;

   /* mock_stmt -> cnst -> ... */
   cnst = tree_child(mock_stmt, 0);

   clen = tree_clen(cnst);
   if (clen != 3) {
      DIE();
      safe_vprintf("<clen> expect = %zu, actual = %zu\n",
         3, clen
      );
   }

   n = tree_chdat(cnst, 0);
   if (n->kind != NODEKIND_ADJ) {
      DIE();
      safe_vprintf("<1st node kind> expect = %d, actual = %d\n",
         NODEKIND_ADJ, n->kind
      );
   }
   if (strcmp(words[0], n->dat.s.run)) {
      DIE();
      safe_vprintf("<1st node data> expect = %s, actual = %s\n",
         words[0], n->dat.s.run
      );
   }

   n = tree_chdat(cnst, 1);
   if (n->kind != NODEKIND_ADJ) {
      DIE();
      safe_vprintf("<2nd node kind> expect = %d, actual = %d\n",
         NODEKIND_ADJ, n->kind
      );
   }
   if (strcmp(words[1], n->dat.s.run)) {
      DIE();
      safe_vprintf("<2nd node data> expect = %s, actual = %s\n",
         words[1], n->dat.s.run
      );
   }

   n = tree_chdat(cnst, 2);
   if (n->kind != NODEKIND_NNOUN) {
      DIE();
      safe_vprintf("<3nd node kind> expect = %d, actual = %d\n",
         NODEKIND_NNOUN, n->kind
      );
   }
   if (strcmp(words[2], n->dat.s.run)) {
      DIE();
      safe_vprintf("<3nd node data> expect = %s, actual = %s\n",
         words[2], n->dat.s.run
      );
   }

   return flag;
}

// TODO: implement type b, c, d, etc
