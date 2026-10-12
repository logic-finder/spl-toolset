#ifndef ARGP_H
#define ARGP_H

#include <stdbool.h>

#include "splc.h"

typedef enum {
   StdoptSpl01,
   StdoptCor27
} stdopt_t;

typedef enum {
   LngoptEn,
   LngoptKo
} lngopt_t;

typedef enum {
   TgtoptC
} tgtopt_t;

typedef struct config {
   bool hlp;
   bool dbg;
   bool kwi;
   bool vsn;
   bool dmp;
   bool opt;
   bool drt;
   bool vbs;
   bool drn;
   unsigned int job;
   char *out;
   stdopt_t std;
   lngopt_t lng;
   tgtopt_t tgt;
   bool asmbly;
   bool disasm;
   bool pdt;
   bool w_kc;
   bool w_bp;
   array_t *source_files;
} config_t;

void _parse_args(compile_ctx_t *cctx);
void destroy_config(void);

extern config_t cfg;

#endif
