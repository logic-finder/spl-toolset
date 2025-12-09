#ifndef ARGPARSE_TYPE_H
#define ARGPARSE_TYPE_H

#include <string.h>
#include <stddef.h>
#include "fatal.h"
#include "global.h"
#include "wrapper.h"
#include "strutil.h"

typedef struct opt {
   char *name;
   void (*handler)(optflg_t *of, optval_t *ov, const char *arg);
} opt_t;

static void parse_endopt(optflg_t *of);
static void parse_filenm(optval_t *ov, const char *arg);
static void parse_shrtop(optflg_t *of, optval_t *_, const char *arg);
static void parse_longop(optflg_t *of, optval_t *ov, const char *arg);

static void handle_exeopt(optflg_t *of, optval_t *_, const char *__);
static void handle_imdopt(optflg_t *of, optval_t *_, const char *__);
static void handle_dscopt(optflg_t *of, optval_t *_, const char *__);
static void handle_kwiopt(optflg_t *of, optval_t *_, const char *__);
static void handle_lngopt(optflg_t *of, optval_t *ov, const char *arg);
static void handle_retopt(optflg_t *of, optval_t *ov, const char *arg);
static void handle_hlpopt(optflg_t *of, optval_t *_, const char *__);
static void handle_vsnopt(optflg_t *of, optval_t *_, const char *__);

#endif
