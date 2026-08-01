#include "wrappers.h"
#include "wrappers.internals.h"

/************
 * stdlib.h *
 ************/
extern void *safe_malloc(size_t siz) {
   void *ret = malloc(siz);
   if (!ret) fatal("malloc error");
   return ret;
}

// fixme: take *siz
extern void *srealloc(void *ptr, size_t siz) {
   ptr = realloc(ptr, siz);
   if (!ptr) fatal("realloc error");
   return ptr;
}

// fixme: take *siz
extern void *srealloc_arr(void *arr, int cnt, size_t esiz) {
   arr = realloc(arr, cnt * esiz);
   if (!arr) fatal("realloc error (arr)");
   return arr;
}

extern void *scalloc(size_t n, size_t siz) {
   void *ret = calloc(n, siz);
   if (!ret) fatal("calloc error");
   return ret;
}

/***********
 * stdio.h *
 ***********/
extern FILE *sfopen(const char *filename, const char *mode) {
   FILE *fp = fopen(filename, mode);
   if (!fp) vfatal("unable to open the file: %s", filename);
   return fp;
}

extern void sfclose(FILE *fp) {
   if (fclose(fp) != EOF)
      return;
   fatal("unable to close a stream");
}

extern void sremove(const char *filename) {
   if (!remove(filename))
      return;
   vfatal("unable to remove the file: %s", filename);
}

extern void srename(const char *old, const char *new) {
   if (!rename(old, new))
      return;
   vfatal("unable to rename %s to %s", old, new);
}

extern int safe_fgetc(FILE *fp) {
   int c = fgetc(fp);
   if (c == EOF && ferror(fp))
      fatal("fgetc error");
   return c;
}

extern void sfputc(FILE *stream, char ch) {
   if (fputc(ch, stream) == EOF)
      fatal("fputc error");
}

extern void sfputs(FILE *stream, const char *line) {
   if (fputs(line, stream) == EOF)
      fatal("fputs error");
}

// fixme: function name and its description at .h
extern void ffmtwrt(FILE *stream, const char *format, ...) {
   va_list ap;
   int ret;

   va_start(ap, format);
   ret = vfprintf(stream, format, ap);
   if (ret < 0) exit(EXIT_FAILURE);
   va_end(ap);
}

// fixme: this also
extern void fmtwrt(const char *format, ...) {
   va_list ap;
   int ret;

   va_start(ap, format);
   ret = vprintf(format, ap);
   if (ret < 0) exit(EXIT_FAILURE);
   va_end(ap);
}

extern void sfwrite(
   void *src,
   size_t siz,
   unsigned int cnt,
   FILE *dest
) {
   if (fwrite(src, siz, cnt, dest) < cnt)
      fatal("fwrite error");
}

extern void sfseek(FILE *fp, long offset, int whence) {
   if (fseek(fp, offset, whence))
      fatal("fseek error");
}

extern void sfgetpos(FILE *fp, fpos_t *pos) {
   if (fgetpos(fp, pos))
      fatal("fgetpos error");
}

extern void sfsetpos(FILE *fp, fpos_t *pos) {
   if (fsetpos(fp, pos))
      fatal("fsetpos error");
}

extern void safe_fprintf(FILE *fp, const char *fmt, int n, ...) {
   va_list ap;
   int ret;  /* vfprintf returns int */

   va_start(ap, n);
   ret = vfprintf(fp, fmt, ap);
   if (ret < n)
      fatal("vfprintf error");
   va_end(ap);
}

extern void safe_fscanf(FILE *fp, const char *fmt, int n, ...) {
   va_list ap;
   int ret;  /* vfscanf returns int */

   va_start(ap, n);
   ret = vfscanf(fp, fmt, ap);
   if (ret != n)
      fatal("vfscanf error");
   va_end(ap);
}
