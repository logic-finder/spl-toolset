#include "lex.h"
#include "lex.type.h"  // contains typedef & prototypes

/******************************
 * IMPORTANT GLOBAL VARIABLES *
 ******************************/
/* Line Access */
extern arr_t *ls;    // array of line_t (see global.h)
static int lls;      // length of ls

static int p;        // line number
static int q;        // position in line
static line_t *l;    // l = arr_peek(ls, p)

static int tp;       // temp. var. for p
static int tq;       // temp. var. for q

/* Line Contents Copy */
static char ch;      // to store a char
static char *buf;    // to store a string
static int idx;      // position in buf
static int max;      // size of buf

/* Miscellaneous */
static jmp_buf LONGJMP_ENV;  // for setjmp & longjmp
static bool eoe;             // end-of-everything
const tokkind_t tokkind;     // kind of token

extern arr_t *lex(
   optflg_t *of,
   optval_t *ov,
   int lc
) {
   // Initialize global variables
   lls = lc;
   p = q = 0;
   l = arr_peek(ls, p);
   max = 128;
   buf = smalloc(max);
   eoe = false;

   // Construct a stream of tokens
   arr_t *toks = arr_create();

   if (!setjmp(env_lex))
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

static void store_token(arr_t *toks) {
   if (idx == 0)
      return;
   store_string(toks, TOKKIND_TOK);
}

static void store_punct(arr_t *toks) {
   if (isspace(buf[0]))
      return;
   store_string(toks, TOKKIND_PNT);
}

static void store_string(arr_t *toks, tokkind_t kind) {
   token_t tok;
   char *run;
   int len;

   buf[idx] = '\0';
   len = idx + 1;
   run = smalloc(len);
   strcpy(run, buf);

   tok.run = run;
   tok.len = len;
   tok.kind = kind;
   tok.lnum = tp;
   tok.lpos = tq;

   arr_append(toks, &tok, sizeof tok);
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
         max *= 2;
         buf = srealloc(buf, max);
      }
      q++;
      return 1;
   }
   else
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
      l = arr_peek(ls, ++p);
   }
   eoe = true;
   end:;
}

static inline void save_state(void) {
   tp = p + 1, tq = q + 1;
}
