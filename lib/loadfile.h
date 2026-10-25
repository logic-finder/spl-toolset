#ifndef LOADFILE_H
#define LOADFILE_H

#include "array.adt.h"
#include "arena.adt.h"

typedef struct line {
   char *run;
   size_t len, num;
} line_t;

array_t *loadfile(
   arena_t *arena,
   const char *filename,
   size_t * restrict lc,
   size_t * restrict wc
);

#endif
