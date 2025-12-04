#include <ctype.h>
#include <stddef.h>
#include "strutil.h"
#include "wrapper.h"

const char *whitespaces = " \n\r\t\a\b\v\f";

extern bool match(char ch, const char *scanset) {
   char cmp;
   bool ret;

   ret = false;
   while ((cmp = *scanset++)) {
      if (ch != cmp)
         continue;
      ret = true;
      break;
   }

   return ret;
}

extern char *extfnm(const char *src, bool ext_flag) {
   /*
    * Note. there are four possible cases for src:
    *    (1) .../name
    *    (2) name
    *    (3) .../
    *    (4) \0
    */
   char dirsep = '/';
   int len = strlen(src);
   int dpos = -1;

   // finds the position of a dirsep from the end
   for (int i = len - 1; i >= 0; i--) {
      if (src[i] != dirsep)
         continue;
      dpos = i;
      break;
   }

   // handles the case 3 and 4
   if (src[dpos + 1] == '\0')
      return NULL;

   // copies src to dest
   char *dest = smalloc(len + 1);

   // handles the case 1 and 2
   if (dpos == -1)
      strcpy(dest, src);
   else
      strcpy(dest, &src[dpos + 1]);

   // requested to keep the extension?
   if (!ext_flag)
      return dest;  /* if not */

   // finds the position of a period from the end
   char *epos = strrchr(dest, '.');

   // there is a period and needs to be removed!
   if (epos) *epos = '\0';

   // now we have a clean basename.
   return dest;
}

// extern char *reducews(const char *src) {
//    char ch, *dest;
//    int p, q, len;
//    bool seen;

//    len = strlen(src);

//    // if (len == 0)
//    //    return NULL;

//    dest = smalloc(len + 1);
//    seen = false;

//    for (p = 0, q = 0; (ch = src[p]); p++) {
//       ch = src[p];
//       if (isspace(ch)) {
//          if (!seen) seen = true;
//          continue;
//       }
//       if (seen) {
//          dest[q++] = ' ';
//          seen = false;
//       }
//       dest[q++] = ch;
//    }
//    dest[q] = '\0';

//    return dest;
// }

// extern char **split(const char *src, char against, int *retsiz) {
//    // sets up an array of strings
//    char **arr;
//    int len, max;

//    len = 0;
//    max = 2;
//    arr = smalloc(max * sizeof (char *));

//    /*
//    against = ','

//    src  ,a,,bc,def,
//    idx  012345678901
//    ini  01 34  7   1
//    fin  0 23  6   01

//    arr = ["", "a", "", "bc", "def", ""]
//    */

//    int i, ini, fin, diff;
//    char ch, *buf;
//    bool seen;

//    ini = fin = 0;
//    seen = false;

//    for (i = 0; ; i++) {
//       ch = src[i];
//       if (ch == against || ch == '\0') {
//          // extracts a substring
//          if (seen) {
//             diff = fin - ini;
//             buf = smalloc(diff + 1);  /* 1 for \0 */
//             strncpy(buf, src + ini, diff);
//          }
//          else {
//             buf = smalloc(1);
//             buf[0] = '\0';
//          }

//          // stores the buffer into the array
//          if (len == max) {
//             max *= 2;
//             arr = srealloc(arr, max);
//          }
//          arr[len++] = buf;

//          // update the states
//          if (ch == '\0')
//             goto end;
//          ini = fin = i + 1;
//          if (seen) seen = false;
//       }
//       else {
//          fin++;
//          if (!seen) seen = true;
//       }
//    }

//    end: *retsiz = len;
//    return arr;
// }

extern char **split(const char *src, char *mark, int *retsiz) {
   // sets up an array of strings
   char **arr;
   int siz, max;

   siz = 0;
   max = 2;
   arr = smalloc(max * sizeof *arr);

   /*
   mark = "**"
   src  = "**a****b**"
   arr  = ["", "a", "", "", "b", ""]
   */

   const int marklen = strlen(mark);
   const char *ini, *fin;
   char *buf;

   ini = fin = src;

   for (;;) {
      // finds the location of the next mark
      fin = strstr(fin, mark);

      if (!fin) {
         int rest = strlen(ini);
         buf = smalloc(rest + 1);
         strcpy(buf, ini);
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

      // stores the buffer into the array
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

// extern void foreach(char **arr, int siz, void (*cb)(char *s, int i)) {
//    for (int i = 0; i < siz; i++)
//       cb(arr[i], i);
// }

extern void trim(char *src) {
   int f, i, len;

   len = strlen(src);

   // finds the index of last non-whitespace char
   for (f = len - 1; f >= 0; f--)
      if (!isspace(src[f]))
         break;

   // removes the trailing wss
   len = f + 1;
   src[len] = '\0';

   // finds the index of first non-ws char
   i = strspn(src, " \a\b\t\n\v\f\r");

   // removes the preceding wss
   len -= i;
   memmove(src, src + i, len + 1);  /* 1 for \0 */
}

extern void translate(char *src, const char *from, const char *to) {
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

   end: strcpy(src, buf);
   free(buf);
}

extern char lastch(const char *line);
