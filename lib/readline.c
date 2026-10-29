#include "readline.h"
#include "readline.internals.h"

extern bool readln(FILE *fp, char **line, size_t *len) {
   int ch;
   size_t pos, bufsiz;
   char *buf;

   /* First of all, checks whether there is nothing to read */
   if ((ch = fgetc(fp)) == EOF) {
      *line = NULL;
      if (len != NULL) *len = 0;
      return false;
   }
   else ungetc(ch, fp);

   bufsiz = READLINE_UNIT;
   if (!(buf = malloc(bufsiz))) {
      return false;
   }
   pos = 0;

   /* Reads the file, one line at a time */
   for (;;) {
      ch = fgetc(fp);

      if (ch == EOF || ch == '\n') {
         break;
      }
      if (ch == '\r') {
         continue;
      }

      buf[pos] = ch;

      if (bufsiz - pos == 1) {
         if (!(buf = realloc2xarr(buf, &bufsiz, ESIZ(buf)))) {
            goto cleanup;
         }
      }

      pos++;
   }
   if (ferror(fp)) {
      goto cleanup;
   }

   /* Stores \n\0 at the end */
   if (bufsiz - pos == 1) {
      if (!(buf = realloc2xarr(buf, &bufsiz, ESIZ(buf)))) {
         goto cleanup;
      }
   }
   buf[pos++] = '\n';
   buf[pos] = '\0';

   *line = buf;
   if (len != NULL) *len = pos;
   return true;

cleanup:
   free(buf);
   return false;
}
