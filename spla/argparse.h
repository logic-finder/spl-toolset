#ifndef ARGPARSE_H
#define ARGPARSE_H

typedef struct optflg {
   unsigned int
      nwn: 1,  // --no-warn
      kwi: 1,  // --kawaii
      eoo: 1,  // --
      hlp: 1,  // --help
      vsn: 1;  // --version
} optflg_t;

typedef struct optval {
   const char *src;
} optval_t;

void parse_args(const char **argv);

#endif
