#include "lex.h"
#include "lex.internals.h"  /* contains typedefs & prototypes */

extern void lex(compile_ctx_t *cctx) {
   lex_ctx_t lctx;
   int ret;

   lctx.toks = array_create(destruct_tok);
   lctx.ls = cctx->ls;
   lctx.lc = cctx->lc;
   lctx.lnum = 0;
   lctx.lpos = 0;
   lctx.l = array_peek(lctx.ls, lctx.lnum);
   lctx.max = 128;
   lctx.buf = safe_malloc(lctx.max);
   lctx.eoe = false;

   safe_fputs(stdout, ENPREFIX "scanning...");

   /* Constructs a stream of tokens */
   ret = setjmp(lctx.env);

   if (ret == 0)
      goto tokenize;
   else
      goto cleanup;

tokenize:
   for (;;) {
      lctx.eoe ? longjmp(lctx.env, 1)
      : skip_space(&lctx);

      lctx.eoe ? longjmp(lctx.env, 2)
      : save_state(&lctx),
        read_token(&lctx),
        store_token(&lctx);

      lctx.eoe ? longjmp(lctx.env, 3)
      : save_state(&lctx),
        read_nchar(&lctx, 1),
        store_punct(&lctx);
   }

cleanup:
   free(lctx.buf);

   safe_vprintf(" " Cgreen "done!" Creset
      "\t(total " Cbwhite "%zu" Creset " tokens)\n",
      array_size(lctx.toks)
   );

   cctx->toks = lctx.toks;
   return;
}

static void destruct_tok(void *item, size_t idx) {
   token_t *tok;

   (void) idx;

   tok = item;
   free(tok->run);
}

static inline void iterate_lines(lex_ctx_t *lctx) {
   int ret;

   while (lctx->lnum < lctx->lc) {
      while (lctx->lpos < lctx->l->len) {
         ret = (*lctx->process)(lctx);
         if (!ret) {
            goto end;
         }
      }
      lctx->lnum++;
      lctx->lpos = 0;
      lctx->l = array_peek(lctx->ls, lctx->lnum);
   }
   lctx->eoe = true;

end:
   return;
}

static int process_skip(lex_ctx_t *lctx) {
   lctx->ch = lctx->l->run[lctx->lpos];

   if ((*lctx->check)(lctx)) {
      lctx->lpos++;
      return 1;
   }

   return 0;
}

static int process_read(lex_ctx_t *lctx) {
   bool last_char;

   lctx->ch = lctx->l->run[lctx->lpos];

   if ((*lctx->check)(lctx)) {
      lctx->buf[lctx->idx] = lctx->ch;

      last_char = (lctx->max - lctx->idx == 1);
      if (last_char) {
         lctx->buf = safe_realloc2x(lctx->buf, &lctx->max);
      }

      lctx->idx++;
      lctx->lpos++;
      return 1;
   }

   return 0;
}

static int check_space(lex_ctx_t *lctx) {
   if (isspace(lctx->ch)) {
      return 1;
   }

   return 0;
}

static int check_cntlessthan(lex_ctx_t *lctx) {
   static size_t cnt = 0;

   if (cnt < lctx->char_limit) {
      cnt++;
      return 1;
   }

   cnt = 0;
   return 0;
}

static int check_token(lex_ctx_t *lctx) {
   static const char *sentinels
      = ".,:[]?! \a\b\t\n\v\f\r";

   if (!match(lctx->ch, sentinels)) {
      return 1;
   }

   return 0;
}

static void skip_space(lex_ctx_t *lctx) {
   lctx->process = process_skip;
   lctx->check = check_space;
   iterate_lines(lctx);
}

static void read_token(lex_ctx_t *lctx) {
   lctx->idx = 0;
   lctx->process = process_read;
   lctx->check = check_token;
   iterate_lines(lctx);
}

static void read_nchar(lex_ctx_t *lctx, size_t n) {
   lctx->idx = 0;
   lctx->process = process_read;
   lctx->check = check_cntlessthan;
   lctx->char_limit = n;
   iterate_lines(lctx);
}

static inline void save_state(lex_ctx_t *lctx) {
   /* actual lnum and lpos starts from the number 1 */
   lctx->real_lnum = lctx->lnum + 1;
   lctx->real_lpos = lctx->lpos + 1;
}

static void store_token(lex_ctx_t *lctx) {
   if (lctx->idx == 0)
      return;
   store_string(lctx, TOKKIND_TOK);
}

static void store_punct(lex_ctx_t *lctx) {
   if (isspace(lctx->buf[0]))
      return;
   store_string(lctx, TOKKIND_PNT);
}

static void store_string(lex_ctx_t *lctx, tokkind_t kind) {
   token_t tok;
   char *buf;
   size_t bufsiz;

   lctx->buf[lctx->idx] = '\0';
   bufsiz = lctx->idx;
   buf = safe_malloc(bufsiz + 1);  /* +1 for \0 */
   memcpy(buf, lctx->buf, bufsiz + 1);  /* \0 copied */

   tok.run = buf;
   tok.len = bufsiz;
   tok.kind = kind;
   tok.lnum = lctx->real_lnum;
   tok.lpos = lctx->real_lpos;

   array_append(lctx->toks, &tok, sizeof tok);
}

/*** DEBUG ***/

extern void showtoks(compile_ctx_t *cctx) {
   array_foreach(cctx->toks, showtok);
}

static void showtok(void *dat, int idx) {
   token_t *tok;

   tok = dat;
   printf("idx = [%d], token = [%s]\n",
      idx, tok->run
   );
}
