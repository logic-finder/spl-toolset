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
static void add_chardecl(parse_ctx_t *pctx, tree_t *dp, size_t cnt, ...);
static void regtok(parse_ctx_t *pctx, const char *run);

/* testcases */
static bool one_person(parse_ctx_t *pctx);
static bool two_person(parse_ctx_t *pctx);

int main(void) {
   parse_ctx_t pctx = {0};
   size_t fail = 0;

   test_title(__FILE__);

   FIXTURE(one_person);
   FIXTURE(two_person);

   test_result(fail);

   return fail == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

static void setup(parse_ctx_t *pctx) {
   token_t dummy = {0};
   tree_t *dp;

   /* Assumes every allocation are successful */
   pctx->arena = arena_create();
   pctx->toks = array_create(NULL, pctx->arena);
   pctx->pt = tree_plant(NULL, 0, pctx->arena);
   pctx->errs = array_create(NULL, pctx->arena);
   pctx->idx = 0;

   graft_tree_n(pctx, pctx->pt, 0, NODEKIND_TITLE, &dummy);
   dp = graft_tree_n(pctx, pctx->pt, 0, NODEKIND_DP, &dummy);

   /* 0 */ add_chardecl(pctx, dp, 1, "Romeo");
   /* 1 */ add_chardecl(pctx, dp, 1, "Juliet");
   /* 2 */ add_chardecl(pctx, dp, 2, "The", "Ghost");
}

static void teardown(parse_ctx_t *pctx) {
   array_destroy(pctx->errs);
   array_destroy(pctx->toks);
   arena_destroy(pctx->arena);
}

static void add_chardecl(parse_ctx_t *pctx, tree_t *dp, size_t cnt, ...) {
   token_t dummy = {0};
   va_list ap;
   tree_t *chardecl;

   chardecl = graft_tree_n(pctx, dp, 0, NODEKIND_CHDECL, &dummy);

   va_start(ap, cnt);
   for (size_t i = 0; i < cnt; i++) {
      char *s = va_arg(ap, char *);
      const token_t t = {
         .run = s,
         .len = strlen(s)
      };
      graft_tree_s(pctx, chardecl, NODEKIND_DATA, &t);
   }
   va_end(ap);
}

static void regtok(parse_ctx_t *pctx, const char *run) {
   token_t tok = {0};

   tok.len = strlen(run);
   /* Assumes these calls do not fail */
   // TODO: it'd be nice to have arena_strdup
   tok.run = arena_malloc(pctx->arena, tok.len + 1);  /* +1 for \0 */
   memcpy(tok.run, run, tok.len + 1);  /* \0 copied */
   (void) array_append(pctx->toks, &tok, sizeof tok);
}

static bool one_person(parse_ctx_t *pctx) {
   /* setup */
   tree_t *mock_base, *enterlike;
   token_t mock_tok = {0};

   mock_base = tree_plant(NULL, 0, pctx->arena);
   enterlike = graft_tree_n(pctx, mock_base, 0, NODEKIND_ENTER, &mock_tok);

   regtok(pctx, "Enter");
   regtok(pctx, "the");
   regtok(pctx, "Ghost");
   regtok(pctx, "]");

   pctx->len = array_size(pctx->toks);
   pctx->tok = array_peek(pctx->toks, 0);

   /* perform */
   parse_namelist(pctx, enterlike);

   /* validate */
   bool flag = true;
   node_t *charnode;
   size_t clen;

   clen = tree_clen(enterlike);
   if (clen != 1) {
      DIE();
      safe_vprintf("<clen> expect = 1, actual = %zu\n", clen);
   }

   charnode = tree_chdat(enterlike, 0);
   if (charnode->dat.n != 2) {  /* 2 = The Ghost */
      DIE();
      safe_vprintf("<1st dp> expect = %d, actual = %d\n",
         2, charnode->dat.n
      );
   }

   return flag;
}

static bool two_person(parse_ctx_t *pctx) {
   /* setup */
   tree_t *mock_base, *enterlike;
   token_t mock_tok = {0};

   mock_base = tree_plant(NULL, 0, pctx->arena);
   enterlike = graft_tree_n(pctx, mock_base, 0, NODEKIND_ENTER, &mock_tok);

   regtok(pctx, "Enter");
   regtok(pctx, "Romeo");
   regtok(pctx, "and");
   regtok(pctx, "Juliet");
   regtok(pctx, "]");

   pctx->len = array_size(pctx->toks);
   pctx->tok = array_peek(pctx->toks, 0);

   /* perform */
   parse_namelist(pctx, enterlike);

   /* validate */
   bool flag = true;
   node_t *charnode;
   size_t clen;

   clen = tree_clen(enterlike);
   if (clen != 2) {
      DIE();
      safe_vprintf("<clen> expect = 1, actual = %zu\n", clen);
   }

   charnode = tree_chdat(enterlike, 0);
   if (charnode->dat.n != 0) {  /* 0 = Romeo */
      DIE();
      safe_vprintf("<1st dp> expect = %d, actual = %d\n",
         0, charnode->dat.n
      );
   }

   charnode = tree_chdat(enterlike, 1);
   if (charnode->dat.n != 1) {  /* 1 = Juliet */
      DIE();
      safe_vprintf("<2nd dp> expect = %d, actual = %d\n",
         1, charnode->dat.n
      );
   }

   return flag;
}

// TODO: implement three_person()
