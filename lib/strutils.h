#ifndef STRUTILS_H
#define STRUTILS_H

#include <string.h>
#include <stdbool.h>

bool match(char ch, const char *scanset);

int match_str(
   const char * restrict s,
   const char ** restrict arr,
   int len
);

char *extfnm(const char *src, bool ext_flag);

char **split(
   const char * restrict src,
   const char * restrict mark,
   int *retsiz
);

void trim(char *src);

void translate(
   char * restrict src,
   const char * restrict from,
   const char * restrict to
);

void normalize(char *src);

/* Inline Functions */
inline char lastch(const char *line) {
   return line[strlen(line) - 1];
}

extern const char *whitespaces;

#endif
