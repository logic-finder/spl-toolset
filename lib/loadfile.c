#include "loadfile.h"
#include "loadfile.internals.h"

extern array_t *loadfile(
   arena_t *arena,
   const char *filename,
   size_t * restrict lc,
   size_t * restrict wc
) {
   FILE *fp;
   array_t *lines;
   line_t line;
   char *run;
   size_t run_len, lcnt, wcnt;

   /* Init */
   lines = NULL;
   wcnt = 0;  /* word count */
   lcnt = 0;  /* line count */

   if ((fp = fopen(filename, "r")) == NULL)
      goto err1;
   if ((lines = array_create(destruct_line, arena)) == NULL)
      goto err2;

   while (!readln(fp, &run, &run_len)) {
      line.len = run_len;
      line.run = run;
      line.num = ++lcnt;
      wcnt += run_len;
      if (!array_append(lines, &line, sizeof line)) {
         goto err3;
      }
   }

err3:
   ;  /* needs to keep 'lines'  */
err2:
   (void) fclose(fp);
err1:
   if (lc != NULL) *lc = lcnt;
   if (wc != NULL) *wc = wcnt;
   return lines;  /* lines was initialized NULL */
}

static void destruct_line(void *item, size_t idx) {
   line_t *l;

   (void) idx;
   l = item;
   free(l->run);
}
