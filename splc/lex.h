#ifndef LEX_H
#define LEX_H

#include "splc.h"
#include "loadfile.h"
#include "argparse.h"
#include "array.adt.h"

/*
 * `TOKKIND_TOK` - a token
 * `TOKKIND_PNT` - a puntuation mark, i.e. `.,:[]?!` and whitespaces
 */
typedef enum tokkind {
   TOKKIND_TOK,
   TOKKIND_PNT
} tokkind_t;

/*
 * `run` - a string
 * `len` - the length of run
 * `tag` - the type of this token. 31 chars + \0
 * `lnum` - the line number to which this token belongs
 * `lpos` - the position in the line
 */
typedef struct token {
   char *run;
   size_t len;
   tokkind_t kind;
   size_t lnum, lpos;
} token_t;

/**
 * `lex` processes the character stream from a source file
 * and produces a token stream.
 * @param cctx a pointer to compile_ctx_t
 */
void lex(compile_ctx_t *cctx);

void showtoks(compile_ctx_t *cctx);

#endif
