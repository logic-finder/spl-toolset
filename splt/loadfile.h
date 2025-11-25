#ifndef LOADFILE_H
#define LOADFILE_H

#include <stdio.h>

typedef struct line {
   char *run;
   int len, num;
} line_t;

line_t *loadfile(const char *filename, int *cnt);
void unloadfl(line_t *lines, int cnt);

#endif
