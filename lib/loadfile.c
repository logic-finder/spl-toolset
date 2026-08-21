#include "loadfile.h"
#include "loadfile.internals.h"

extern array_t *loadfile(
   const char *filename,
   size_t * restrict lc,
   size_t * restrict wc
) {
   FILE *fp;
   array_t *lines;
   line_t line;
   size_t rlen,   /* the length of `run` */
          lcnt,   /* line count */
          wcnt;   /* word count */
   char *run;  /* the contents of a line */

   fp = safe_fopen(filename, "r");
   lcnt = wcnt = 0;
   lines = array_create();

   while (!readln(fp, &run, &rlen)) {
      line.len = rlen;
      line.run = run;
      line.num = ++lcnt;
      wcnt += rlen;
      array_append(lines, &line, sizeof line);
   }

   if (lcnt == 0) {
      fatal("empty source file");
   }

   safe_fclose(fp);

   if (lc != NULL) *lc = lcnt;
   if (wc != NULL) *wc = wcnt;
   return lines;
}

extern void unloadfl(array_t *lines, size_t lc) {
   line_t *l;

   for (size_t i = 0; i < lc; i++) {
      l = array_peek(lines, i);
      free(l->run);
   }
   array_destroy(lines);
}
