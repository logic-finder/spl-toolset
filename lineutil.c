#include <ctype.h>
#include <stdbool.h>
#include "fatal.h"
#include "wrapper.h"
#include "lineutil.h"

static char *errmsg = "%s: unable to read.";

extern int readln(FILE *fp, char **line, int *len) {
   // tests whether there is nothing to read
   int ch;

   ch = getc(fp);
   if (ch == EOF) {
      if (ferror(fp)) vfatal(errmsg, __func__);
      *line = NULL;
      *len = 0;
      return 1;
   }
   else ungetc(ch, fp);

   // reads chars from the file so as to construct a string
   int bufsiz = READLINE_UNIT;
   char *buf = smalloc(bufsiz);
   int pos;
   bool eol;

   pos = 0;
   eol = false;

   for (;;) {
      if (eol) {
         buf[pos] = '\0';
         *line = buf;
         *len = pos;
         return 0;
      }

      ch = getc(fp);

      if (ch == EOF) {
         if (ferror(fp)) vfatal(errmsg, __func__);
         buf[pos] = '\0';
         *line = buf;
         *len = pos;
         return 0;
      }
      if (ch == '\r')
         continue;
      if (ch == '\n') //{
         eol = true;
         //continue;
      //}

      buf[pos++] = ch;

      if (pos == bufsiz) {
         bufsiz *= 2;
         buf = srealloc(buf, bufsiz);
      }
   }
}

extern int skipws(FILE *fp) {
   int ch;

   while (isspace(ch = fgetc(fp)))
      /* empty loop body */ ;

   if (ferror(fp))
      fatal("read error occurred!");
   if (feof(fp))
      return EOF;

   ungetc(ch, fp);
   return 0;
}

extern int skipln(FILE *fp) {
   int ch;

   while ((ch = fgetc(fp)) != EOF)
      if (ch == '\n')
         break;
   if (ferror(fp))
      fatal("read error occurred!");

   return feof(fp) ? EOF : 0;
}
