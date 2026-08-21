#ifndef LEX_INTERNALS_H
#define LEX_INTERNALS_H

#include <ctype.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdbool.h>

#include "msg.h"
#include "global.h"
#include "strutils.h"
#include "wrappers.h"
#include "colorcode.h"

/*==========*
 | TYPEDEFS |
 *==========*/
typedef struct {
   array_t *ls;  /* array of line_t */
   size_t lc;    /* length of ls */
   size_t lnum;  /* line number */
   size_t lpos;  /* column in line */
   line_t *l;    /* a line */
   size_t tlnum;  /* temporary lnum */
   size_t tlpos;  /* temporary lpos */
   char ch;    /* a char */
   char *buf;  /* a buffer */
   size_t idx;  /* column in buf */
   size_t max;  /* max size of buf */
   jmp_buf env;  /* setjmp & longjmp */
   bool eoe;  /* end of everything */
} lex_ctx_t;

typedef int processor_t(lex_ctx_t *lctx, va_list *ap);
typedef int checker_t(lex_ctx_t *lctx, va_list *ap);

/*=====================*
 | FUNCTION PROTOTYPES |
 *=====================*/

/* Character Handling */
static void skip_space(lex_ctx_t *lctx);
static void read_token(lex_ctx_t *lctx);
static void read_nchar(lex_ctx_t *lctx, size_t n);

/* Utils */
static inline void save_state(lex_ctx_t *lctx);
static void store_token(lex_ctx_t *lctx, array_t *toks);
static void store_punct(lex_ctx_t *lctx, array_t *toks);

/* Miscellaneous */
static inline void iterate_lines(lex_ctx_t *lctx, processor_t *process, ...);
static processor_t process_skip;
static processor_t process_read;
static checker_t check_space;
static checker_t check_cntlessthan;
static checker_t check_token;
static void store_string(lex_ctx_t *lctx, array_t *toks, tokkind_t kind);

#endif
