#include "lex.h"
#include "lex.internals.h"  /* contains typedefs & prototypes */

extern bool lex(compile_ctx_t *cctx) {
   lex_ctx_t lctx = {
      .ls = cctx->ls,
      .lc = cctx->lc,
      .lnum = 0,
      .lpos = 0,
      .l = array_peek(cctx->ls, 0),
      .max = 128,
      .eoe = false,
      .toks = NULL
   };
   bool ret = true;

   lctx.toks = array_create(destruct_tok, cctx->arena);
   if (lctx.toks == NULL) {
      ret = false;
      goto OOM1;
   }
   lctx.buf = safe_malloc(lctx.max);
   if (lctx.buf == NULL) {
      ret = false;
      goto OOM2;
   }

   if (cctx->cfg->vbs) safe_fputs(stdout, ENPREFIX "scanning...");

   /* Constructs a stream of tokens */
   for (;;) {
      if (lctx.eoe) break;
      else skip_space(&lctx);

      if (lctx.eoe) break;
      else {
         save_state(&lctx);
         read_token(&lctx);
         store_token(&lctx);
      }

      if (lctx.eoe) break;
      else {
         save_state(&lctx);
         read_nchar(&lctx, 1);
         store_punct(&lctx);
      }
   }

   if (cctx->cfg->vbs) safe_vprintf(" " Cgreen "done!" Creset
      "\t(total " Cbwhite "%zu" Creset " tokens)\n",
      array_size(lctx.toks)
   );

   /* cleanup */
   free(lctx.buf);
OOM2:
   ;  /* need to keep lctx.toks */
OOM1:
   cctx->toks = lctx.toks;
   return ret;
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

[[maybe_unused]]
extern void showtoks(compile_ctx_t *cctx) {
   array_foreach(cctx->toks, showtok, NULL);
}

static void showtok(void *dat, size_t idx, void *ctx) {
   token_t *tok;

   (void) ctx;

   tok = dat;
   printf("idx = [%zu], token = [%s]\n",
      idx, tok->run
   );
}
