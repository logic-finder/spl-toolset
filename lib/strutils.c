#include "strutils.h"
#include "strutils.internals.h"

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
   char *dest = smalloc(len + 1);

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

// split하면 구분자도 있어야되는데 구분자 포함 안되는 구현이기 때문에
// 함수명을 바꾸든지 해야할듯
// 구분자도 포함하려면 ini=fin, ini=fin=fin+marklen을 ='\0' 밑에 각각 배치
extern char **split(
   const char * restrict src,
   const char * restrict mark,
   int *retsiz
) {
   /*
    * An example input & output
    * mark = "**"
    * src  = "**a****b**"
    * arr  = ["a", "b"]
    */
   const int marklen = strlen(mark);

   int siz, max;
   const char *ini, *fin;
   char *buf, **arr;

   // Prepare an array of strings
   siz = 0;
   max = 2;
   arr = smalloc(max * sizeof arr[0]);
   ini = fin = src;

   for (;;) {
      // find the location of the next mark
      fin = strstr(fin, mark);

      if (!fin) {
         int rest = strlen(ini);
         buf = smalloc(rest + 1);
         strcpy(buf, ini); // fixme: consider memcpy
      }
      else {
         ptrdiff_t diff = fin - ini;
         if (!diff) {
            buf = smalloc(1);
            buf[0] = '\0';
         }
         else {
            buf = smalloc(diff + 1);
            strncpy(buf, ini, diff);
            buf[diff] = '\0';
         }
      }

      // store the buffer into the array
      if (siz == max) {
         max *= 2;
         arr = srealloc(arr, max);
      }
      arr[siz++] = buf;

      // update the states
      if (!fin)
         goto end;
      ini = fin = fin + marklen;
   }
   end: *retsiz = siz;

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

   buf = smalloc(strlen(src) + 1);
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

extern char lastch(const char *line);
