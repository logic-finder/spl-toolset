#include "fatal.h"
#include "wrapper.h"

/************
 * stdlib.h *
 ************/
extern void *smalloc(size_t siz) {
   void *ret = malloc(siz);
   if (!ret) fatal("malloc error");
   return ret;
}

extern void *srealloc(void *ptr, size_t siz) {
   ptr = realloc(ptr, siz);
   if (!ptr) fatal("realloc error");
   return ptr;
}

extern void *srealloc_arr(void *arr, int cnt, size_t esiz) {
   arr = realloc(arr, cnt * esiz);
   if (!arr) fatal("realloc error (arr)");
   return arr;
}

extern void *scalloc(size_t n, size_t siz) {
   void *ret = calloc(n, siz);
   if (!ret) fatal("calloc error.");
   return ret;
}

/***********
 * stdio.h *
 ***********/
extern FILE *sfopen(const char *filename, const char *mode) {
   FILE *fp = fopen(filename, mode);
   if (!fp) vfatal("unable to open the file: %s.", filename);
   return fp;
}

extern void sfclose(FILE *fp) {
   if (fclose(fp) != EOF)
      return;
   fatal("unable to close a stream.");
}

extern void sfputs(FILE *stream, const char *line) {
   if (fputs(line, stream) == EOF)
      fatal("fputs error.");
}

extern void ffmtwrt(FILE *stream, const char *format, ...) {
   va_list ap;
   int ret;

   va_start(ap, format);
   ret = vfprintf(stream, format, ap);
   if (ret < 0) exit(EXIT_FAILURE);
   va_end(ap);
}

extern void fmtwrt(const char *format, ...) {
   va_list ap;
   int ret;

   va_start(ap, format);
   ret = vprintf(format, ap);
   if (ret < 0) exit(EXIT_FAILURE);
   va_end(ap);
}
