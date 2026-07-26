#ifndef WRAPPERS_H
#define WRAPPERS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/**
 * @brief `smalloc` (safe malloc) is a wrapper for `malloc`.
 * @note exits in failure.
 */
void *smalloc(size_t siz);

/**
 * @brief `srealloc` (safe realloc) is a wrapper for `realloc`.
 * @note exits in failure.
 */
void *srealloc(void *ptr, size_t siz);

/**
 *
 */
void *srealloc_arr(void *arr, int cnt, size_t esiz);

/**
 * @brief `scalloc` (safe calloc) is a wrapper for `calloc`.
 * @note exits in failure.
 */
void *scalloc(size_t n, size_t siz);

/**
 * @brief `sfopen` (safe fopen) is a wrapper for `fopen`.
 * @note exits in failure.
 */
FILE *sfopen(const char *filename, const char *mode);

/**
 * @brief `sfclose` (safe fclose) is a wrapper for `fclose`.
 * @note exits in failure.
 */
void sfclose(FILE *fp);

void sremove(const char *filename);
void srename(const char *old, const char *new);

/**
 *
 */
void sfputc(FILE *stream, char ch);

/**
 * @brief `sfputs` (safe fputs) is a wrapper for `fputs`.
 * @note exits in failure.
 */
void sfputs(FILE *stream, const char *line);

/**
 * @brief `ffmtwrt` (file format write) is a wrapper for `fprintf`.
 * @note exits in failure.
 */
void ffmtwrt(FILE *stream, const char *format, ...);

/**
 * @brief `fmtwrt` (format write) is a wrapper for `printf`.
 * @note exits in failure.
 */
void fmtwrt(const char *format, ...);

/**
 *
 */
void sfwrite(
   void *src,
   size_t siz,
   unsigned int cnt,
   FILE *dest
);

/**
 *
 */
void sfseek(FILE *fp, long offset, int whence);

/**
 *
 */
void sfgetpos(FILE *fp, fpos_t *pos);

/**
 *
 */
void sfsetpos(FILE *fp, fpos_t *pos);

#endif
