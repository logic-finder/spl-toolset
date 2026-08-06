#include "strutils.h"
#include "strutils.internals.h"

/* Note: refer to strutils.h for the inline definition */
extern char lastch(const char *line);

extern bool match(char ch, const char *scanset) {
   while (*scanset)
      if (ch == *scanset++)
         return true;
   return false;
}

extern size_t match_str(
   const char *  restrict str,
   const char ** restrict arr,
   size_t arrlen
) {
   for (size_t i = 0; i < arrlen; i++)
      if (!strcmp(str, arr[i]))
         return i;
   return arrlen;
}

extern char *basename(const char *src) {
   /* There are four possible cases for src:
         (1) .../name
         (2) name
         (3) .../
         (4) \0   */

   size_t len;  /* the length of src */
   char *dpos,  /* position of '/' */
        *dest;  /* a buffer to hold the result */

   /* Finds the position of '/' from the end */
   len = strlen(src);

   for (dpos = src + len - 1; dpos >= src; dpos--)
      if (*dpos == '/')
         break;

   /* case 3 & 4 */
   if (*(dpos + 1) == '\0')
      return NULL;

   /* case 1 & 2 */
   dest = safe_malloc(len + 1);  /* max size */
   strcpy(dest, dpos + 1);

   return dest;
}

extern char *stem(const char *src) {
   char *ret, *cpos;

   ret = basename(src);
   if (!ret)
      return NULL;

   cpos = strrchr(ret, '.');
   if (cpos)
      *cpos = '\0';

   return ret;
}

extern char **split(
   const char * restrict src,
   const char * restrict mark,
   size_t *retsiz
) {
   /* EXAMPLE USAGE
         mark = "**"
         input  = "**a****b**"
         output = ["a", "b"] */

   /* Declares variables */
   const int marklen = strlen(mark);

   char **arr;  /* result array */
   size_t siz;     /* the current size of arr */
   size_t max;     /* the max size of arr */

   /* 0123456789012
      I like tacos!
             ^    ^
           ini    fin
      diff = fin - ini
         = 12 - 7
         = 5
         = strlen(tacos) */

   const char *ini, *fin;
   ptrdiff_t diff;

   char *buf;  /* a buffer to copy a substring */
   size_t rest;    /* a length of a string */

   /* Prepares an array of strings */
   siz = 0;
   max = 2;
   arr = safe_malloc(max * ESIZ(arr));

   /* Parsing */
   ini = src;
   for (;;) {
      /* Searches the location of the next mark */
      fin = strstr(ini, mark);

      /* No mark found? */
      if (!fin) {
         /* since there is no more mark, copies
            the rest of the string into buf */
         rest = strlen(ini);
         buf = safe_malloc(rest + 1);
         memcpy(buf, ini, rest + 1);  /* includes \0 */
         break;
      }

      /* Copies the substring into buf */
      diff = fin - ini;  /* equals the token length */
      buf = safe_malloc(diff + 1);  /* +1 for \0 */
      memcpy(buf, ini, diff);
      buf[diff] = '\0';

      /* Stores the buffer into the array */
      if (siz == max) {
         max *= 2;
         arr = safe_realloc2x(arr, max);
      }
      arr[siz++] = buf;

      /* Updates parsing states */
      ini = fin + marklen;  /* skips the mark found */
   }

   *retsiz = siz;
   return arr;
}

extern void trim(char *src) {
   char *fin, *ini;
   size_t len;

   len = strlen(src);

   /* Searches for a non-whitespace character from the back */
   for (fin = src + len - 1; fin >= src; fin--)
      if (!isspace(*fin))
         break;

   /* Removes the trailing whitespaces */
   *(fin + 1) = '\0';

   if (fin == src - 1)  /* when src consists of only whitespaces */
      return;

   /* Finds the first non-whitespace character */
   ini = src + strspn(src, whitespaces);

   /* Removes the preceding whitespaces by moving ini to src */
   len = strlen(ini);
   memmove(src, ini, len + 1);  /* +1 for \0 */
}

extern void translate(
   char * restrict src,
   const char * restrict from,
   const char * restrict to
) {
   char ch, *pos;
   ptrdiff_t idx;

   for (size_t i = 0; (ch = src[i]); i++) {
      pos = strchr(from, ch);
      if (!pos) continue;
      idx = pos - from;
      src[i] = to[idx];
   }
}

extern void normalize(char *src) {
   char *tok;

   tok = strtok(src, whitespaces);
   if (!tok)
      return;

   do {
      *tok = ' ';
      tok = strtok(NULL, whitespaces);
   } while (!tok);
}

extern char *make_destname(
   const char * restrict orig,
   const char * restrict ext
) {
   char *temp, *buf;
   size_t tempsiz;

   temp = stem(orig);
   if (!temp)
      fatal("filename is empty");
   tempsiz = strlen(temp);
   buf = safe_malloc(tempsiz + strlen(ext) + 1);
   strcpy(buf, temp);
   strcat(buf, ext);

   return buf;
}
