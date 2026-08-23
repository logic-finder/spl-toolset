#ifndef DBMAKER_INTERNALS_H
#define DBMAKER_INTERNALS_H

#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>

#include "fatal.h"
#include "common.h"
#include "global.h"
#include "wrappers.h"
#include "strutils.h"
#include "array.adt.h"
#include "db.common.h"
#include "readline.h"
#include "colorcodes.h"

#define SRCFILE(sectname, idx)      \
safe_vprintf(                             \
   "   " Cgreen "%s" Creset         \
   " for " #sectname " section\n",  \
      ov.mak[idx]                   \
   )

#define DONE(idx, tabs)                              \
safe_vprintf(                                              \
      #tabs Cbgreen "done!" Creset                   \
      " (total " Cbwhite "%d" Creset " entries)\n",  \
      ecnts[idx]                                     \
   )

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
      safe_vprintf(                                         \
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
   array_sorter_t *compare
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
   array_sorter_t *compare
);

static uint32_t write_metadata(FILE *fp);
static writer_t write_namesect;
static writer_t write_adjsect;
static writer_t write_nounsect;
static writer_t write_compsect;

static array_sorter_t compare_name;
static array_sorter_t compare_adj;
static array_sorter_t compare_noun;
static array_sorter_t compare_comp;

static void print_srcfiles(void);
static void handle_dberr(void);

static array_destructor_t destruct_record_A;
static array_destructor_t destruct_record_B;

static uint32_t ecnts[SECTNUM];
static bool le, be;
static bool dupflg;

#endif
