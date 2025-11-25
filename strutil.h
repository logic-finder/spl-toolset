#ifndef STRUTIL_H
#define STRUTIL_H

#include <stdbool.h>

#define ROMAN_NUM "IVXLCDM"

bool match(char ch, const char *scanset);

// remove path and optionally extension
char *extfnm(const char *src, bool ext_flag);

//char *reducews(const char *src);

//char **split(const char *src, char against, int *retsiz);

extern char **split(const char *src, char *mark, int *retsiz);

//void foreach(char **arr, int siz, void (*cb)(char *s, int i));

extern void trim(char *src);

#endif
