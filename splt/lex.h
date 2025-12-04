#ifndef LEX_H
#define LEX_H

#include "list.adt.h"
#include "loadfile.h"
#include "argparse.h"

#define TOK_TOK "TOKEN"
#define TOK_PNT "PUNCT"

list_t *lex(
   optflg_t *of,
   optval_t *ov,
   line_t *arr,
   int len
);

#endif
