#ifndef LOADFILE_H
#define LOADFILE_H

#include "array.adt.h"

typedef struct line {
   char *run;
   int len, num;
} line_t;

array_t *loadfile(
   const char *filename,
   int * restrict lc,
   int * restrict wc
);
void unloadfl(array_t *lines, int cnt);

#endif
