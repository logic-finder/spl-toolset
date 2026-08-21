#include "lex.h"
#include "lex.internals.h"  /* contains typedefs & prototypes */

extern array_t *lex(optflg_t *of, optval_t *ov, size_t lc) {
   (void) of, (void) ov;

   // Initialize global variables
   lls = lc;
   p = q = 0;
   l = array_peek(ls, p);
   max = 128;
   buf = safe_malloc(max);
   eoe = false;

   // Construct a stream of tokens
   array_t *toks = array_create();

   if (!setjmp(LONGJMP_ENV))
      goto tokenize;
   else
      goto cleanup;

   tokenize:
   for (;;) {
      eoe ? JUMP(1)
      : skip_space();

      eoe ? JUMP(1)
      : save_state(),
        read_token(),
        store_token(toks);

      eoe ? JUMP(1)
      : save_state(),
        read_nchar(1),
        store_punct(toks);
   }

   cleanup:
   free(buf);

   return toks;
}

static void store_token(array_t *toks) {
   if (idx == 0)
      return;
   store_string(toks, TOKKIND_TOK);
}

static void store_punct(array_t *toks) {
   if (isspace(buf[0]))
      return;
   store_string(toks, TOKKIND_PNT);
}

static void store_string(array_t *toks, tokkind_t kind) {
   token_t tok;
   char *run;
   int len;

   buf[idx] = '\0';
   len = idx + 1;
   run = safe_malloc(len);
   strcpy(run, buf); // fixme: consider memcpy

   tok.run = run;
   tok.len = len;
   tok.kind = kind;
   tok.lnum = tp;
   tok.lpos = tq;

   array_append(toks, &tok, sizeof tok);
}

static void skip_space(void) {
   iterate_lines(process_skip, check_space);
}

static void read_token(void) {
   idx = 0;
   iterate_lines(process_read, check_token);
}

static void read_nchar(int n) {
   idx = 0;
   iterate_lines(process_read, check_cntlessthan, n);
}

static int process_skip(va_list *ap) {
   checker_t *check;

   check = va_arg(*ap, checker_t *);
   ch = l->run[q];

   if ((*check)(ap)) {
      q++;
      return 1;
   }
   else
      return 0;
}

static int process_read(va_list *ap) {
   checker_t *check;

   check = va_arg(*ap, checker_t *);
   ch = l->run[q];

   if ((*check)(ap)) {
      buf[idx++] = ch;
      if (idx == max) {
         buf = safe_realloc2x(buf, &max);
      }
      q++;
      return 1;
   }
   else
      /*
       * buf[idx] = '\0';
       * if it were here, buf would not be null-
       * terminated when we reach the end of `ls`
       */
      return 0;
}

static int check_space(va_list *_) {
   (void) _;
   if (isspace(ch)) return 1;
   else return 0;
}

static int check_cntlessthan(va_list *ap) {
   static int cnt = 0;
   int n;

   n = va_arg(*ap, int);
   if (cnt < n) {
      cnt++;
      return 1;
   }
   else {
      cnt = 0;
      return 0;
   }
}

static int check_token(va_list *_) {
   static const char *sentinels
      = ".,:[]?! \a\b\t\n\v\f\r";

   (void) _;
   if (!match(ch, sentinels)) return 1;
   else return 0;
}

static inline void iterate_lines(processor_t *process, ...) {
   va_list ap;
   int ret;

   while (p < lls) {
      while (q < l->len) {
         va_start(ap, process);
         ret = (*process)(&ap);
         va_end(ap);
         if (!ret) goto end;
      }
      q = 0;
      l = array_peek(ls, ++p);
   }
   eoe = true;
end: ;
}

static inline void save_state(void) {
   tp = p + 1, tq = q + 1;
}
