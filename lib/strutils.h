#ifndef STRUTILS_H
#define STRUTILS_H

#include <string.h>
#include <stdbool.h>

typedef struct [[nodiscard]] {
   const char **alikes;
   size_t nmemb;
} find_alikes_ret_t;

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
[[nodiscard]]
char *basename(const char *src);

/* need to free the return value later */
[[nodiscard]]
char *stem(const char *src);

[[nodiscard]]
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

[[nodiscard]]
char *make_destname(
   const char * restrict orig,
   const char * restrict ext
);

int strcasecmp(const char *s1, const char *s2);

[[nodiscard]]
char *tolower_str(const char *orig);

[[nodiscard]]
char *strdup(const char *src);

[[nodiscard]]
char *strndup(const char *src, size_t n);

bool is_rnum(const char *rnum);

unsigned int interpret_romnum(const char *romnum);

[[nodiscard]]
int levenshtein_distance(const char *s, const char *t);

[[nodiscard]]
find_alikes_ret_t find_alikes(
   const char *src,
   const void **targets,
   size_t targets_len,
   size_t target_siz
);

/* Inline Functions */
inline char lastch(const char *line) {
   size_t len = strlen(line);
   return (len != 0) ? line[len - 1] : '\0';
}

#endif
