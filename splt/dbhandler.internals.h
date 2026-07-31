#ifndef DB_INTERNALS_H
#define DB_INTERNALS_H

#include <stdint.h>
#include "msg.h"
#include "global.h"
#include "common.h"
#include "arr.adt.h"
#include "wrappers.h"
#include "db.common.h"
#include "colorcode.h"

typedef int comparer_t(const void *key, const void *elem);

static void dbcheck(void);

static void check_secthead(
   sectkind_t kind,
   int hdsiz,
   uint32_t against
);

static void read_ecnt(sectkind_t kind, int ecntsiz);

static void load_section(sectkind_t kind, size_t esiz);

static comparer_t compare_rec_A;
static comparer_t compare_rec_B;

static inline void dberr(const char *reason);

static uint32_t secpos[SECTNUM];
static uint32_t ecnts[SECTNUM];
static void *sects[SECTNUM];
static bool le, be;
static FILE *db;

#endif
