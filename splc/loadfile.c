// fixme: lib으로 옮기기

#include "loadfile.h"
#include "loadfile.internals.h"

extern arr_t *loadfile(
   const char *filename,
   int * restrict lc,
   int * restrict wc
) {
   FILE *fp;
   arr_t *lines;
   line_t line;
   char *run;  // the contents of a line
   int rlen,   // the length of `run`
       lcnt,   // line count
       wcnt;   // word count

   fp = safe_fopen(filename, "r");
   lcnt = wcnt = 0;
   lines = arr_create();

   while (!readln(fp, &run, &rlen)) {
      line.len = rlen;
      line.run = run;
      line.num = ++lcnt;
      wcnt += rlen;
      arr_append(lines, &line, sizeof line);
   }

   if (lcnt == 0)
      ERR("empty source file");

   line_t *last = arr_peek(lines, lcnt - 1);
   char *r = last->run;

   // fixme: readln에서 애초에 모든 줄 끝에 \n을 붙이도록 하고
   // 이부분 삭제
   if (lastch(r) != '\n') {
      r = last->run = srealloc(r, last->len + 1);
      strcat(r, "\n");  /* \n\0 */
      last->len++;
   }

   safe_fclose(fp);

   // fixme: if (lc) *lc = lcnt... 식으로 변경
   *lc = lcnt;
   *wc = wcnt;
   return lines;
}

extern void unloadfl(arr_t *lines, int lc) {
   for (int i = 0; i < lc; i++)
      free(((line_t *) arr_peek(lines, i))->run);
   arr_destroy(lines);
}
