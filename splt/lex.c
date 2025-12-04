#include "lex.h"
#include "lex.type.h"  // contains typedef & prototypes

/******************************
 * IMPORTANT GLOBAL VARIABLES *
 ******************************/
/* Line Access */
line_t *ls;   // array of line_t
static int lls;      // length of ls

static int p;        // line number
static int q;        // position in line
static line_t *l;    // l = &ls[p]

static int tp;       // temp. var. for p
static int tq;       // temp. var. for q

/* Line Contents Copy */
static char ch;      // to store a char
static char *buf;    // to store a string
static int idx;      // position in buf
static int max;      // size of buf

/* Miscellaneous */
static bool eoe_indicator = false;  // end-of-everything
const char *sfname;   // name of source file
static jmp_buf lex_env;

extern list_t *lex(
   optflg_t *of,
   optval_t *ov,
   line_t *arr,
   int len
) {
   // Initialize global variables
   ls = arr;
   lls = len;
   p = q = 0;
   l = &ls[p];
   max = 128;
   buf = smalloc(max);
   sfname = ov->src;

   // Construct a stream of tokens
   list_t *toks = list_create();

   if (!setjmp(lex_env))
      goto tokenize;
   else
      goto cleanup;

   tokenize:
   for (;;) {
      eoe()
         ? longjmp(lex_env, 1)
         : skip_space();
      eoe()
         ? longjmp(lex_env, 2)
         : save_state(),
           read_token(),
           store_token(toks);
      eoe()
         ? longjmp(lex_env, 3)
         : save_state(),
           read_nchar(1),
           store_punct(toks);
   }

   cleanup:
   free(buf);

   return toks;
}

static void store_token(list_t *toks) {
   if (idx == 0)
      return;
   store_string(toks, TOK_TOK);
}

static void store_punct(list_t *toks) {
   if (isspace(buf[0]))
      return;
   store_string(toks, TOK_PNT);
}

static void store_string(list_t *toks, const char *type) {
   buf[idx] = '\0';
   (void) list_push(toks, buf, idx + 1, type, tp, tq);
}

static void skip_space(void) {
   iterate_lines(process_skip, check_space);

}

static void skip_nchar(int n) {
   iterate_lines(process_skip, check_cntlessthan, n);
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
   else {
      //buf[idx] = '\0';
      return 0;
   }
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
         ret = process(&ap);
         va_end(ap);
         if (!ret) goto end;
      }
      q = 0;
      l = ls + ++p;
   }
   eoe_indicator = true;
   end:;
}

static void lexerr(teller_t *tell) {
   err_template(tell, Cbblue, "<lexer error> ");
}

static void tell_eoe(void) {
   l = ls + --p;
   ffmtwrt(stderr,
      "%s\n"
      "[%s:%d:%d] " Cbwhite "note:" Creset " reached end of source file\n"
      "%4d| %s" Cbblack "EOF" Creset "\n",
      msgs.err.lex_eof,
      sfname, l->num, l->len,
      l->num, l->run
   );
}

static bool eoe(void) {
   return eoe_indicator ? true : false;
   // if (p < lls - 1)
   //    return false;
   // if (q < l->len - 1)
   //    return false;
   // return true;
}

static inline void save_state(void) {
   tp = p + 1, tq = q + 1;
}
