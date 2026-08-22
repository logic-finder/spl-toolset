#ifndef LEX_INTERNALS_H
#define LEX_INTERNALS_H

#include <ctype.h>
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
typedef struct tag_lex_ctx_t lex_ctx_t;
typedef int processor_t(lex_ctx_t *lctx);
typedef int checker_t(lex_ctx_t *lctx);

struct tag_lex_ctx_t {
   array_t *toks;  /* array of token_t */
   array_t *ls;    /* array of line_t */
   size_t lc;    /* length of ls */
   size_t lnum;  /* line number */
   size_t lpos;  /* column in line */
   line_t *l;    /* a line */
   size_t real_lnum;  /* actual lnum */
   size_t real_lpos;  /* actual lpos */
   char ch;    /* a char */
   char *buf;  /* a buffer */
   size_t idx;  /* column in buf */
   size_t max;  /* max size of buf */
   jmp_buf env;  /* setjmp & longjmp */
   bool eoe;  /* end of everything */
   processor_t *process;
   checker_t *check;
   size_t char_limit;  /* used in read_nchar() */
};

/*=====================*
 | FUNCTION PROTOTYPES |
 *=====================*/
static void destruct_tok(void *item, size_t idx);
static array_iterator_t showtok;

/* Character Handling */
static void skip_space(lex_ctx_t *lctx);
static void read_token(lex_ctx_t *lctx);
static void read_nchar(lex_ctx_t *lctx, size_t n);

/* Utils */
static inline void save_state(lex_ctx_t *lctx);
static void store_token(lex_ctx_t *lctx);
static void store_punct(lex_ctx_t *lctx);

/* Miscellaneous */
static inline void iterate_lines(lex_ctx_t *lctx);
static processor_t process_skip;
static processor_t process_read;
static checker_t check_space;
static checker_t check_cntlessthan;
static checker_t check_token;
static void store_string(lex_ctx_t *lctx, tokkind_t kind);

#endif
