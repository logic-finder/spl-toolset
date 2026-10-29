#ifndef READLINE_H
#define READLINE_H

#include <stdio.h>
#include <stdbool.h>

#define READLINE_UNIT 128

bool readln(FILE *fp, char **line, size_t *len);

#endif
