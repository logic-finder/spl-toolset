#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <stdbool.h>

typedef struct optflg {
   unsigned int
      exe: 1,  // --exe
      imd: 1,  // --keep-intermediate
      dsc: 1,  // --describe
      kwi: 1,  // --kawaii
      lng: 1,  // --lang
      ret: 1,  // --ret
      eoo: 1,  // --
      hlp: 1,  // --help
      vsn: 1;  // --version
} optflg_t;

typedef struct optval {
   const char *ret, *lng, *src;
} optval_t;

void parse_args(const char **argv, optflg_t *of, optval_t *ov);

#endif
