#ifndef STRUTIL_H
#define STRUTIL_H

#include <string.h>
#include <stdbool.h>

extern const char *whitespaces;

bool match(char ch, const char *scanset);
char *extfnm(const char *src, bool ext_flag);
extern char **split(
   const char * restrict src,
   const char * restrict mark,
   int *retsiz
);
extern void trim(char *src);
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

#endif
