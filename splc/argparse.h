#ifndef ARGPARSE_H
#define ARGPARSE_H

#include "splc.h"

struct optflg {
   unsigned int
      exe: 1,  /* --exe */
      imd: 1,  /* --keep-intermediate */
      dsc: 1,  /* --describe */
      kwi: 1,  /* --kawaii */
      lng: 1,  /* --lang */
      ret: 1,  /* --ret */
      dbg: 1,  /* -g */
      opt: 1,  /* -O */
      dmp: 1,  /* -S */
      eoo: 1,  /* -- */
      pdt: 1,  /* --pedantic */
      w_bp: 1, /* --Wbe-predicate */
      w_kc: 1, /* --Wkeyword-case */
      tgt: 1,  /* --target */
      drn: 1,  /* --dry-run */
      std: 1,  /* --std */
      hlp: 1,  /* --help */
      vsn: 1;  /* --version */
};

struct optval {
   const char *ret, *lng, *src, *tgt, *std;
};

void parse_args(compile_ctx_t *cctx);
void destroy_options(compile_ctx_t *cctx);

extern const char *stdopt_spl01;
extern const char *stdopt_cor27;

#endif
