#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include "fatal.h"

// fixme: move ERR and VERR into fatal.h
#define ERR(msg) fatal(ENPREFIX msg)
#define VERR(msg, ...) vfatal(ENPREFIX msg, __VA_ARGS__)
#
#define ESIZ(arr) (sizeof (arr)[0])
#define ARRLEN(arr) (sizeof (arr) / ESIZ(arr))

bool isle(void);

inline uint32_t endrev32(uint32_t v) {
   return v << 24
         | (v & 0x0000FF00) << 8
         | (v & 0x00FF0000) >> 8
         | (v & 0xFF000000) >> 24;
}

#endif
