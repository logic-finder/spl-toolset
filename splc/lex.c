#include "lex.h"
#include "lex.internals.h"  /* contains typedefs & prototypes */

extern array_t *lex(optflg_t *of, optval_t *ov, array_t *ls, size_t lc) {
   lex_ctx_t lctx;
   int ret;

   (void) of, (void) ov;

   lctx.ls = ls;
   lctx.lc = lc;
   lctx.lnum = 0;
   lctx.lpos = 0;
   lctx.l = array_peek(lctx.ls, lctx.lnum);
   lctx.max = 128;
   lctx.buf = safe_malloc(lctx.max);
   lctx.eoe = false;

   /* Constructs a stream of tokens */
   array_t *toks = array_create();

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
        store_token(&lctx, toks);

      lctx.eoe ? longjmp(lctx.env, 3)
      : save_state(&lctx),
        read_nchar(&lctx, 1),
        store_punct(&lctx, toks);
   }

cleanup:
   free(lctx.buf);

   return toks;
}

static void store_token(lex_ctx_t *lctx, array_t *toks) {
   if (lctx->idx == 0)
      return;
   store_string(lctx, toks, TOKKIND_TOK);
}

static void store_punct(lex_ctx_t *lctx, array_t *toks) {
   if (isspace(lctx->buf[0]))
      return;
   store_string(lctx, toks, TOKKIND_PNT);
}

static void store_string(lex_ctx_t *lctx, array_t *toks, tokkind_t kind) {
   token_t tok;
   char *run;
   size_t len;

   lctx->buf[lctx->idx] = '\0';
   len = lctx->idx + 1;
   run = safe_malloc(len);
   strcpy(run, lctx->buf); // fixme: consider memcpy

   tok.run = run;
   tok.len = len;
   tok.kind = kind;
   tok.lnum = lctx->tlnum;
   tok.lpos = lctx->tlpos;

   array_append(toks, &tok, sizeof tok);
}

static void skip_space(lex_ctx_t *lctx) {
   iterate_lines(lctx, process_skip, check_space);
}

static void read_token(lex_ctx_t *lctx) {
   lctx->idx = 0;
   iterate_lines(lctx, process_read, check_token);
}

static void read_nchar(lex_ctx_t *lctx, size_t n) {
   lctx->idx = 0;
   iterate_lines(lctx, process_read, check_cntlessthan, n);
}

static int process_skip(lex_ctx_t *lctx, va_list *ap) {
   checker_t *check;

   check = va_arg(*ap, checker_t *);
   lctx->ch = lctx->l->run[lctx->lpos];

   if ((*check)(lctx, ap)) {
      lctx->lpos++;
      return 1;
   }
   else
      return 0;
}

static int process_read(lex_ctx_t *lctx, va_list *ap) {
   checker_t *check;
   bool last_char;

   check = va_arg(*ap, checker_t *);
   lctx->ch = lctx->l->run[lctx->lpos];

   if ((*check)(lctx, ap)) {
      lctx->buf[lctx->idx] = lctx->ch;

      last_char = (lctx->max - lctx->idx == 1);
      if (last_char) {
         lctx->buf = safe_realloc2x(lctx->buf, &lctx->max);
      }

      lctx->idx++;
      lctx->lpos++;
      return 1;
   }
   else {
      /* BEWARE OF THE BUG!
         buf[idx] = '\0';
      if it were here, buf would not be null-
      terminated when we reach the end of ls */
      return 0;
   }
}

static int check_space(lex_ctx_t *lctx, va_list *ap) {
   (void) ap;

   if (isspace(lctx->ch)) {
      return 1;
   }

   return 0;
}

static int check_cntlessthan(lex_ctx_t *lctx, va_list *ap) {
   static size_t cnt = 0;
   size_t n;

   (void) lctx;

   n = va_arg(*ap, size_t);

   if (cnt < n) {
      cnt++;
      return 1;
   }

   cnt = 0;
   return 0;
}

static int check_token(lex_ctx_t *lctx, va_list *ap) {
   static const char *sentinels
      = ".,:[]?! \a\b\t\n\v\f\r";

   (void) ap;

   if (!match(lctx->ch, sentinels)) {
      return 1;
   }

   return 0;
}

static inline void iterate_lines(lex_ctx_t *lctx, processor_t *process, ...) {
   va_list ap;
   int ret;

   while (lctx->lnum < lctx->lc) {
      while (lctx->lpos < lctx->l->len) {
         va_start(ap, process);
         ret = (*process)(lctx, &ap);
         va_end(ap);
         if (!ret) {
            goto end;
         }
      }
      lctx->lnum++;
      lctx->lpos = 0;
      lctx->l = array_peek(ls, lctx->lnum);
   }
   lctx->eoe = true;

end:
   return;
}

static inline void save_state(lex_ctx_t *lctx) {
   lctx->tlnum = lctx->lnum + 1;
   lctx->tlpos = lctx->lpos + 1;
}
