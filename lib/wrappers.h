#ifndef WRAPPERS_H
#define WRAPPERS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef void safe_malloc_cb_t(int errcode, size_t sz);

void safe_malloc_set_callback(safe_malloc_cb_t *cb);

/**
 * @brief `safe_malloc` is a wrapper for `malloc`.
 * @note exits in failure.
 */
[[deprecated]]
void *safe_malloc(size_t siz);

/**
 * @brief `safe_calloc` is a wrapper for `calloc`.
 * @note exits in failure.
 */
[[deprecated]]
void *safe_calloc(size_t n, size_t siz);

/**
 * @brief `safe_realloc2x` is a wrapper for `realloc`.
 * @note exits in failure.
 */
[[deprecated]]
inline void *safe_realloc2x(void *orig, size_t *siz);

/**
 *
 */
[[deprecated]]
void *safe_realloc2x_arr(void *orig_arr, size_t *cnt, size_t esiz);

void *realloc2xarr(void *orig_arr, size_t *cnt, size_t esiz);

/**
 * @brief `safe_fopen` is a wrapper for `fopen`.
 * @note exits in failure.
 */
[[deprecated]]
FILE *safe_fopen(const char *filename, const char *mode);

/**
 * @brief `safe_fclose` is a wrapper for `fclose`.
 * @note exits in failure.
 */
[[deprecated]]
void safe_fclose(FILE *fp);

[[deprecated]]
void safe_remove(const char *filename);

[[deprecated]]
void safe_rename(const char *old, const char *new);

[[deprecated]]
int safe_fgetc(FILE *fp);

/**
 *
 */
[[deprecated]]
void safe_fputc(FILE *stream, char ch);

/**
 * @brief `safe_fputs` is a wrapper for `fputs`.
 * @note exits in failure.
 */
[[deprecated]]
void safe_fputs(FILE *stream, const char *line);

/**
 * @brief `safe_vfprintf` is a wrapper for `vfprintf`.
 * @note exits in failure.
 */
[[deprecated]]
void safe_vfprintf(FILE *stream, const char *format, ...);

/**
 * @brief `safe_vprintf` is a wrapper for `vprintf`.
 * @note exits in failure.
 */
[[deprecated]]
void safe_vprintf(const char *format, ...);

[[deprecated]]
void safe_vprintf2(const char *format, va_list ap);

/**
 *
 */
[[deprecated]]
void safe_fwrite(
   void *src,
   size_t siz,
   unsigned int cnt,
   FILE *dest
);

/**
 *
 */
[[deprecated]]
void safe_fseek(FILE *fp, long offset, int whence);

/**
 *
 */
[[deprecated]]
void safe_fgetpos(FILE *fp, fpos_t *pos);

/**
 *
 */
[[deprecated]]
void safe_fsetpos(FILE *fp, fpos_t *pos);

[[deprecated]]
void safe_fprintf(FILE *fp, const char *fmt, ...);

[[deprecated]]
void safe_fscanf(FILE *fp, const char *fmt, int n, ...);

/*====================*
 | INLINE DEFINITIONS |
 *====================*/
[[deprecated]]
inline void *safe_realloc2x(void *orig, size_t *siz) {
   return safe_realloc2x_arr(orig, siz, sizeof (char));
}

#endif
