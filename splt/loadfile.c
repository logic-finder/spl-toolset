#include "wrapper.h"
#include "loadfile.h"
#include "lineutil.h"

#define INIT_SIZE 128

extern line_t *loadfile(const char *filename, int *cnt) {
   FILE *fp;

   fp = sfopen(filename, "r");

   line_t *lines;  // an array to contain lines
   char *run;  // the contents of a line
   int len;  // the length of the line
   int num;  // the line number of it
   int max;

   num = 0;
   max = INIT_SIZE;
   lines = smalloc(max * sizeof *lines);

   while (!readln(fp, &run, &len)) {
      if (num == max) {
         max *= 2;
         lines = srealloc(lines, max);
      }
      lines[num].len = len;
      lines[num].run = run;
      lines[num].num = num + 1;
      num++;
   }

   // fixme: 막줄에 줄바꿈 없으면 줄바꿈 넣기 (재할당 필요할듯)

   sfclose(fp);

   if (num == 0) {
      free(lines);
      *cnt = 0;
      return NULL;
   }
   *cnt = num;
   return lines;
}

extern void unloadfl(line_t *lines, int cnt) {
   for (int i = 0; i < cnt; i++)
      free(lines[i].run);
   free(lines);
}
