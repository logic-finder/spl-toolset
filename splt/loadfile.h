#ifndef LOADFILE_H
#define LOADFILE_H

#include "arr.adt.h"

typedef struct line {
   char *run;
   int len, num;
} line_t;

arr_t *loadfile(
   const char *filename,
   int * restrict lc,
   int * restrict wc
);
void unloadfl(arr_t *lines, int cnt);

#endif
