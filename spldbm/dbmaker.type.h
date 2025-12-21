#ifndef DBMAKER_TYPE_H
#define DBMAKER_TYPE_H

#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include "common.h"
#include "global.h"
#include "wrapper.h"
#include "dbmaker.h"
#include "strutil.h"
#include "arr.adt.h"
#include "lineutil.h"
#include "colorcode.h"

#define SRCFILE(sectname, idx)      \
ffmtwrt(stdout,                     \
   "   " Cgreen "%s" Creset         \
   " for " #sectname " section\n",  \
      ov.mak[idx]                   \
   );
#
#define DONE(idx, tabs)                              \
ffmtwrt(stdout,                                      \
      #tabs Cbgreen "done!" Creset                   \
      " (total " Cbwhite "%d" Creset " entries)\n",  \
      ecnts[idx]                                     \
   );
#
#define GENERIC_COMPARE(type)                         \
static int compare_rec_##type(                        \
   const void *_lhs,                                  \
   const void *_rhs,                                  \
   const char *sect                                   \
) {                                                   \
   record_##type##_t * const *lhs, * const * rhs;     \
   int ret;                                           \
                                                      \
   lhs = _lhs, rhs = _rhs;                            \
   ret = strcmp((*lhs)->run, (*rhs)->run);            \
   if (!ret) {                                        \
      ffmtwrt(stdout,                                 \
         "\n" Cbred "<DB error>" Creset               \
         " the same %s entry " Cbmagenta "%s" Creset  \
         " detected at line " Cbwhite "%d" Creset     \
         " and " Cbwhite "%d" Creset,                 \
         sect, (*lhs)->run,                           \
         (*lhs)->lnum, (*rhs)->lnum                   \
      );                                              \
      dupflg = true;                                  \
   }                                                  \
                                                      \
   return ret;                                        \
}

typedef struct record_A {
   uint8_t len;
   char *run;
   int lnum;
} record_A_t;

typedef struct record_B {
   uint8_t kind;
   uint8_t len;
   char *run;
   int lnum;
} record_B_t;

typedef void writer_t(FILE *fp);

static void write_sect_type_A(
   FILE *fp,
   const char *sectname,
   sectkind_t kind,
   uint32_t header,
   int hd_len,
   int ec_len,
   int dt_l_len,
   int dt_s_len,
   arr_sorter_t *compare
);
static void write_sect_type_B(
   FILE *fp,
   const char *sectname,
   sectkind_t kind,
   uint32_t header,
   int hd_len,
   int ec_len,
   int dt_k_len,
   int dt_l_len,
   int dt_s_len,
   arr_sorter_t *compare
);

static uint32_t write_metadata(FILE *fp);
static writer_t write_namesect;
static writer_t write_adjsect;
static writer_t write_nounsect;
static writer_t write_compsect;

static arr_sorter_t compare_name;
static arr_sorter_t compare_adj;
static arr_sorter_t compare_noun;
static arr_sorter_t compare_comp;

static void print_srcfiles(void);
static void handle_dberr(void);

#endif
