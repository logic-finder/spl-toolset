#ifndef STRUTILS_H
#define STRUTILS_H

#include <string.h>
#include <stdbool.h>

bool match(char ch, const char *scanset);

/* Returns arrlen when str doesn't exist in arr */
size_t match_str(
   const char *  restrict str,
   const char ** restrict arr,
   size_t arrlen
);

size_t match_str_case(
   const char *  restrict str,
   const char ** restrict arr,
   size_t arrlen
);

/* need to free the return value later */
char *basename(const char *src);

/* need to free the return value later */
char *stem(const char *src);

char **split(
   const char * restrict src,
   const char * restrict mark,
   size_t *retsiz
);

/* modifies src */
void trim(char *src);

void translate(
   char * restrict src,
   const char * restrict from,
   const char * restrict to
);

/* modifies src */
void normalize(char *src);

char *make_destname(
   const char * restrict orig,
   const char * restrict ext
);

int strcasecmp(const char *s1, const char *s2);

char *tolower_str(const char *orig);

char *strdup(const char *src);

/* Inline Functions */
inline char lastch(const char *line) {
   return line[strlen(line) - 1];
}

#endif
