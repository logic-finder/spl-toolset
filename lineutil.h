#ifndef LINEUTIL_H
#define LINEUTIL_H

#include <stdio.h>

#define READLINE_UNIT   64

int readln(FILE *fp, char **line, int *len);

#endif
