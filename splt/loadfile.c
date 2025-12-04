#include "global.h"
#include "wrapper.h"
#include "loadfile.h"
#include "lineutil.h"

#define INIT_SIZE 128

extern line_t *loadfile(const char *filename, int *lc, int *wc) {
   FILE *fp;

   fp = sfopen(filename, "r");

   line_t *lines;  // an array to contain lines
   char *run;  // the contents of a line
   int len;  // the length of the line
   int linecount;  // the line number of it
   int wordcount;
   int max;

   linecount = wordcount = 0;
   max = INIT_SIZE;
   lines = smalloc(max * sizeof *lines);

   while (!readln(fp, &run, &len)) {
      if (linecount == max) {
         max *= 2;
         lines = srealloc(lines, max);
      }
      lines[linecount].len = len;
      lines[linecount].run = run;
      lines[linecount].num = linecount + 1;
      linecount++;
      wordcount += len;
   }

   // fixme: 막줄에 줄바꿈 없으면 줄바꿈 넣기 (재할당 필요할듯)
   // 애초에 처음 할당받을때 한줄을 더 받으면 되지 않을까?

   sfclose(fp);

   if (linecount == 0) {
      // free(lines);
      // *cnt = 0;
      // return NULL;
      ERR("source file empty");
   }
   *lc = linecount;
   *wc = wordcount;
   return lines;
}

extern void unloadfl(line_t *lines, int cnt) {
   for (int i = 0; i < cnt; i++)
      free(lines[i].run);
   free(lines);
}
