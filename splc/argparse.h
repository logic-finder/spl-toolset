#ifndef ARGPARSE_H
#define ARGPARSE_H

#include "splc.h"

struct optflg {
   unsigned int
      eoo: 1,  /* -- */
      hlp: 1,  /* --help */
      dbg: 1,  /* --debug */
      kwi: 1,  /* --kawaii */
      vsn: 1,  /* --version */
      dmp: 1,  /* --dump */
      opt: 1,  /* --optimize */
      drt: 1,  /* --direct */
      vbs: 1,  /* --verbose */
      drn: 1,  /* --dry-run */
      job: 1,  /* --j */
      out: 1,  /* --o */
      std: 1,  /* --std */
      lng: 1,  /* --lang */
      tgt: 1,  /* --target */
      asmbly: 1, /* --asm */
      disasm: 1, /* --disasm */
      pdt: 1,  /* --pedantic */
      w_bp: 1, /* --Wbe-predicate */
      w_kc: 1  /* --Wkeyword-case */
};

struct optval {
   const char *src;
   const char *job;
   const char *out;
   const char *std;
   const char *lng;
   const char *tgt;
};

void parse_args(compile_ctx_t *cctx);
void destroy_options(compile_ctx_t *cctx);

extern const char *stdopt_spl01;
extern const char *stdopt_cor27;

#endif
