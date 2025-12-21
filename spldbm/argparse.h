#ifndef ARGPARSE_H
#define ARGPARSE_H

typedef struct optflg {
   unsigned int
      mak: 1,  // --make
      arc: 1,  // --archive
      res: 1,  // --restore
      kwi: 1,  // --kawaii
      hlp: 1,  // --help
      vsn: 1;  // --version
} optflg_t;

typedef struct optval {
   char **mak;
   const char *arc, *res;
} optval_t;

void parse_args(const char **argv);

#endif
