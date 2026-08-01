#include "lineutils.h"
#include "lineutils.internals.h"

static const char *errmsg = "%s: unable to read";

// fixme: 파일명 lineutils에서 readline으로 바꿀 것

extern int readln(FILE *fp, char **line, int *len) {
   // Test whether there is nothing to read
   int ch, pos, bufsiz;
   bool eol;
   char *buf;

   ch = getc(fp);
   if (ch == EOF) {
      if (ferror(fp)) vfatal(errmsg, __func__);
      *line = NULL;
      *len = 0;
      return 1;
   }
   else ungetc(ch, fp);

   // Read chars from the file so as to construct a string
   bufsiz = READLINE_UNIT;
   buf = safe_malloc(bufsiz);
   pos = 0;
   eol = false;

   for (;;) {
      if (eol) {
// fixme: append \n at eol if there isn't
   end_of_line:
         buf[pos] = '\0';
         *line = buf;
         *len = pos;  /* i.e. len == strlen(buf) + 1 */
         return 0;
      }
// fixme: use getc wrapper
      ch = getc(fp);

      if (ch == EOF) {
// fixme: eliminate this ferror with using of getc wrapper
         if (ferror(fp)) vfatal(errmsg, __func__);
         goto end_of_line;
      }
      if (ch == '\r')
         continue;
      if (ch == '\n')
         eol = true;

      buf[pos++] = ch;

      if (pos == bufsiz) {
         bufsiz *= 2;
         buf = srealloc(buf, bufsiz);
      }
   }
}
