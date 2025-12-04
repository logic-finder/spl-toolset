#ifndef GLOBAL_H
#define GLOBAL_H

#include "fatal.h"
#include "loadfile.h"

#define EXECNAME "splt"
#define ENPREFIX EXECNAME ": "
#
#define ERR(msg) fatal(ENPREFIX msg)
#define VERR(msg, ...) vfatal(ENPREFIX msg, __VA_ARGS__)
#
#define ARRLEN(arr) (sizeof (arr) / sizeof (arr)[0])

typedef void teller_t(void);

void err_template(teller_t *tell, const char *color, const char *msg);

extern line_t *ls;
extern const char *sfname;

#endif
