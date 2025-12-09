#include "loadfile.h"
#include "loadfile.type.h"

extern arr_t *loadfile(const char *filename, int *lc, int *wc) {
   FILE *fp;
   arr_t *lines;
   char *run;  // the contents of a line
   int llen;   // the length of the line
   int lcnt;   // line count
   int wcnt;   // word count
   line_t line;

   fp = sfopen(filename, "r");
   lcnt = wcnt = 0;
   lines = arr_create();

   while (!readln(fp, &run, &llen)) {
      line.len = llen;
      line.run = run;
      line.num = ++lcnt;
      wcnt += llen;
      arr_append(lines, &line, sizeof line);
   }

   if (lcnt == 0)
      ERR("empty source file");

   line_t *last = arr_peek(lines, lcnt - 1);
   char *r = last->run;

   if (lastch(r) != '\n') {
      r = last->run = srealloc(r, last->len + 1);
      strcat(r, "\n");  /* \n\0 */
      last->len++;
   }

   sfclose(fp);

   *lc = lcnt;
   *wc = wcnt;
   return lines;
}

extern void unloadfl(arr_t *lines, int lc) {
   for (int i = 0; i < lc; i++)
      free(((line_t *) arr_peek(lines, i))->run);
   free(lines);
}
