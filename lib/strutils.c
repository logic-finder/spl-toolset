#include "strutils.h"
#include "strutils.internals.h"

extern char lastch(const char *line);

extern bool match(char ch, const char *scanset) {
   while (*scanset)
      if (ch == *scanset++)
         return true;
   return false;
}

extern int match_str(
   const char * restrict s,
   const char ** restrict arr,
   int len
) {
   for (int i = 0; i < len; i++)
      if (!strcmp(s, arr[i]))
         return i;
   return -1;
}

extern char *extfnm(const char *src, bool ext_flag) {
   /*
    * There are four possible cases for src:
    *    (1) .../name
    *    (2) name
    *    (3) .../
    *    (4) \0
    */
   static const char dirsep = '/';

   int len, dpos;
   char *epos;

   // Find the position of a dirsep from the end
   len = strlen(src);
   dpos = -1;

   for (int i = len - 1; i >= 0; i--) {
      if (src[i] != dirsep)
         continue;
      dpos = i;
      break;
   }

   // Handle the case 3 and 4
   if (src[dpos + 1] == '\0')
      return NULL;

   // Copy src to dest
   char *dest = safe_malloc(len + 1);

   // Handle the case 1 and 2
   if (dpos == -1)
      strcpy(dest, src); // fixme: consider memcpy
   else
      strcpy(dest, &src[dpos + 1]); // fixme: consider memcpy

   // Requested to keep the extension?
   if (ext_flag)
      return dest;

   // Remove the extension
   epos = strrchr(dest, '.');
   if (epos)
      *epos = '\0';

   return dest;
}

extern char **split(
   const char * restrict src,
   const char * restrict mark,
   int *retsiz
) {
   /* EXAMPLE
         mark = "**"
         input  = "**a****b**"
         output = ["a", "b"] */

   const int marklen = strlen(mark);

   ptrdiff_t diff;
   int siz, max;
   const char *ini, *fin;
   char *buf, **arr;

   /* Prepares an array of strings */
   siz = 0;
   max = 2;
   // fixme: use ESIZ
   arr = safe_malloc(max * sizeof arr[0]);
   ini = fin = src;

   do {
      /* Searches the location of the next mark */
      fin = strstr(fin, mark);

      if (!fin) {
         int rest = strlen(ini);
         buf = safe_malloc(rest + 1);
         strcpy(buf, ini); // fixme: consider memcpy
      }
      else {
         diff = fin - ini;
         if (!diff) {
            buf = safe_malloc(1);
            buf[0] = '\0';
         }
         else {
            buf = safe_malloc(diff + 1);
            strncpy(buf, ini, diff);
            buf[diff] = '\0';
         }
      }

      /* Stores the buffer into the array */
      if (siz == max) {
         max *= 2;
         arr = safe_realloc2x(arr, max);
      }
      arr[siz++] = buf;

      /* Updates parsing states */
      ini = fin = fin + marklen;
   } while (fin);  /* Escapes this do-while when fin == NULL */

   *retsiz = siz;
   return arr;
}

extern void trim(char *src) {
   int f, i, len;

   len = strlen(src);

   // Find the index of last non-whitespace char
   for (f = len - 1; f >= 0; f--)
      if (!isspace(src[f]))
         break;

   // Remove the trailing whitespaces
   len = f + 1;
   src[len] = '\0';

   // Find the index of first non-ws char
   i = strspn(src, " \a\b\t\n\v\f\r");

   // Remove the preceding whitespaces
   len -= i;
   memmove(src, src + i, len + 1);  /* 1 for \0 */
}

extern void translate(
   char * restrict src,
   const char * restrict from,
   const char * restrict to
) {
   char ch, *pos;
   ptrdiff_t idx;

   for (int i = 0; (ch = src[i]); i++) {
      pos = strchr(from, ch);
      if (!pos) continue;
      idx = pos - from;
      src[i] = to[idx];
   }
}

extern void normalize(char *src) {
   char *buf, *tok;

   buf = safe_malloc(strlen(src) + 1);
   buf[0] = '\0';

   tok = strtok(src, whitespaces);
   if (!tok) {
      strcat(buf, " ");
      goto end;
   }

   while (tok) {
      strcat(buf, tok);
      strcat(buf, " ");
      tok = strtok(NULL, whitespaces);
   }
   buf[strlen(buf) - 1] = '\0';

end:
   strcpy(src, buf); // fixme: consider memcpy
   free(buf);
}

extern char *make_destname(
   const char * restrict orig,
   const char * restrict ext
) {
   char *temp, *buf;
   size_t tempsiz;

   temp = extfnm(orig, false);
   tempsiz = strlen(temp);
   buf = safe_malloc(tempsiz + strlen(ext) + 1);
   strcpy(buf, temp);
   strcat(buf, ext);

   return buf;
}
