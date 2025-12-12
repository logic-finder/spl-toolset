#ifndef GLOBAL_H
#define GLOBAL_H

#include "lex.h"
#include "fatal.h"
#include "loadfile.h"

/**********
 * MACROS *
 **********/
#define EXECNAME "splt"
#define ENPREFIX EXECNAME ": "
#
#define ERR(msg) fatal(ENPREFIX msg)
#define VERR(msg, ...) vfatal(ENPREFIX msg, __VA_ARGS__)
#
#define ESIZ(arr) (sizeof (arr)[0])
#define ARRLEN(arr) (sizeof (arr) / ESIZ(arr))
#
#define LONGJMP_TEMPLET(e,v) longjmp((e),(v))

/************
 * TYPEDEFS *
 ************/
typedef void teller_t(void);

/***********************
 * FUNCTION PROTOTYPES *
 ***********************/
void err_template(
   teller_t *tell,
   const char * restrict color,
   const char * restrict msg
);

/**********************
 * EXTERNAL VARIABLES *
 **********************/
extern const char *sfname;       // from optprocessor.c
extern arr_t *ls;                // from splt.c

#endif
