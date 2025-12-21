#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include "fatal.h"

#define ERR(msg) fatal(ENPREFIX msg)
#define VERR(msg, ...) vfatal(ENPREFIX msg, __VA_ARGS__)
#
#define ESIZ(arr) (sizeof (arr)[0])
#define ARRLEN(arr) (sizeof (arr) / ESIZ(arr))
#
#define DBFILENAME "words.spldb"
#define DBTEMPNAME DBFILENAME ".temp"
#
#define HEADER_METADATA 0x53504442UL
#define HEADER_NAMESECT 0x4E414D45UL
#define HEADER_ADJSECT  0x41444A20UL
#define HEADER_NOUNSECT 0x4E4F554EUL
#define HEADER_COMPSECT 0x434F4D50UL
#
#define SECPOS_LEN 4
#
#define MTDT_HD 4
#define MTDT_AF 1
#define MTDT_SP 4
#define MTDT_SIZ (MTDT_HD + MTDT_AF + MTDT_SP * SECPOS_LEN)
#
#define NAME_HD 4
#define NAME_EC 4
#define NAME_DT_L 1
#define NAME_DT_S 64
#define NAME_MTDTSIZ (NAME_HD + NAME_EC)
#define NAME_DTSIZ (NAME_DT_L + NAME_DT_S)
#
#define ADJ_HD 4
#define ADJ_EC 4
#define ADJ_DT_L 1
#define ADJ_DT_S 64
#define ADJ_MTDTSIZ (ADJ_HD + ADJ_EC)
#define ADJ_DTSIZ (ADJ_DT_L + ADJ_DT_S)
#
#define NOUN_HD 4
#define NOUN_EC 4
#define NOUN_DT_K 1
#define NOUN_DT_L 1
#define NOUN_DT_S 64
#define NOUN_MTDTSIZ (NOUN_HD + NOUN_EC)
#define NOUN_DTSIZ (NOUN_DT_K + NOUN_DT_L + NOUN_DT_S)
#
#define COMP_HD 4
#define COMP_EC 4
#define COMP_DT_K 1
#define COMP_DT_L 1
#define COMP_DT_S 64
#define COMP_MTDTSIZ (COMP_HD + COMP_EC)
#define COMP_DTSIZ (COMP_DT_K + COMP_DT_L + COMP_DT_S)

typedef enum sectkind {
   SECTKIND_NAME,
   SECTKIND_ADJ,
   SECTKIND_NOUN,
   SECTKIND_COMP
} sectkind_t;

typedef enum arcflg {
   Arcflg_f, Arcflg_t
} arcflg_t;

bool isle(void);

inline uint32_t endrev32(uint32_t v) {
   return v << 24
         | (v & 0x0000FF00) << 8
         | (v & 0x00FF0000) >> 8
         | (v & 0xFF000000) >> 24;
}

#endif
