// fixme: lib으로 옮기기

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
      ERR("empty source file");
   }

   line_t *last = array_peek(lines, lcnt - 1);
   char *r = last->run;

   // fixme: readln에서 애초에 모든 줄 끝에 \n을 붙이도록 하고
   // 이부분 삭제
   if (lastch(r) != '\n') {
      r = last->run = safe_realloc2x(r, last->len + 1);
      strcat(r, "\n");  /* \n\0 */
      last->len++;
   }

   safe_fclose(fp);

   if (lc != NULL) *lc = lcnt;
   if (wc != NULL) *wc = wcnt;
   return lines;
}

extern void unloadfl(array_t *lines, size_t lc) {
   line_t *l;

   for (int i = 0; i < lc; i++) {
      l = array_peek(lines, i);
      free(l->run);
   }
   array_destroy(lines);
}
