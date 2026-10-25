#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

#include "array.adt.h"

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
   array_t *source_files;
   bool hlp;
   bool dbg;
   bool kwi;
   size_t vsn;
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
} config_t;

bool init_config(config_t *cfg);
void destroy_config(config_t *cfg);
void apply_config(config_t *cfg);

extern config_t cfg;

#endif
