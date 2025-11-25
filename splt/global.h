#ifndef GLOBAL_H
#define GLOBAL_H

#include "fatal.h"

#define EXECNAME "splt"
#define ENPREFIX EXECNAME ": "

#define ERR(msg) fatal(ENPREFIX msg)
#define VERR(msg, ...) vfatal(ENPREFIX msg, __VA_ARGS__)

#endif
