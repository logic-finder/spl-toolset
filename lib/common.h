#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define ESIZ(arr) (sizeof (arr)[0])
#define ARRLEN(arr) (sizeof (arr) / ESIZ(arr))

// fixme: 변수타입으로 쓰지 말고, 변수는 그냥 sizet 같은거 쓰되 경계값을 넘는지를 검사하도록 하자
// 그편이 프로그램 작성이 편할거같음
#define SPL_INT_SIZ 4
#
#if     SPL_INT_SIZ == 4
#define SPL_INT_MAX INT32_MAX
#define SPL_INT_MIN INT32_MIN
#define SPL_INT_FMTSPC      PRId32
#define SPL_UINT_FMTSPC     PRIu32
#define SPL_UINT_FMTSPC_HEX PRIX32
#elif   SPL_INT_SIZ == 8
#define SPL_INT int64_t
#define SPL_INT_MAX INT64_MAX
#define SPL_INT_MIN INT64_MIN
#define SPL_INT_FMTSPC      PRId64
#define SPL_UINT_FMTSPC     PRIu64
#define SPL_UINT_FMTSPC_HEX PRIX64
#else
#error undefined int size
#endif
#
#if   SPL_INT_SIZ == 4
typedef int32_t  spl_int_t;
typedef uint32_t spl_uint_t;
#elif SPL_INT_SIZ == 8
typedef int64_t  spl_int_t;
typedef uint64_t spl_uint_t;
#endif

bool isle(void);

size_t min(size_t a, size_t b);

size_t min3(size_t a, size_t b, size_t c);

inline uint32_t endrev32(uint32_t v) {
   return v << 24
         | (v & 0x0000FF00) << 8
         | (v & 0x00FF0000) >> 8
         | (v & 0xFF000000) >> 24;
}

#endif
