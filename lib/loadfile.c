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
   bool fclosed;

   /* Inits locals */
   lines = NULL;
   wcnt = 0;  /* word count */
   lcnt = 0;  /* line count */
   fclosed = false;

   /* Opens the file to read */
   if (!(fp = fopen(filename, "r"))) {
      goto err1;
   }

   /* Makes an array to contain lines */
   if (!(lines = array_create(destruct_line, arena))) {
      goto err2;
   }

   /* Until the end of file,
         - reads a line
         - stores it in the array */
   while (!readln(fp, &run, &run_len)) {
      line.len = run_len;
      line.run = run;
      line.num = ++lcnt;
      wcnt += run_len;
      if (!array_append(lines, &line, sizeof line)) {
         goto err3;
      }
   }

   if (fclose(fp) == EOF) {
      fclosed = true;
      goto err4;
   }
   if (lc) *lc = lcnt;
   if (wc) *wc = wcnt;
   return lines;

err4:
   ;
err3:
   arena_free_fallback(arena, lines);
err2:
   if (!fclosed) (void) fclose(fp);
err1:
   return NULL;
}

static void destruct_line(void *item, size_t idx) {
   line_t *l;

   (void) idx;
   l = item;
   free(l->run);
}
