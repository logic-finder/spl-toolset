#ifndef LEX_H
#define LEX_H

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
 * @param of option flag
 * @param ov option value
 * @param lc the length of ls
 */
array_t *lex(
   optflg_t *of,
   optval_t *ov,
   array_t *ls,
   size_t lc
);

#endif
