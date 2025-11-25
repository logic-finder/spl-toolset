#ifndef LINEUTIL_H
#define LINEUTIL_H

#include <stdio.h>

#define READLINE_UNIT   64

// does not store \n
int readln(FILE *fp, char **line, int *len);
int skipws(FILE *fp);
int skipln(FILE *fp);

#endif
