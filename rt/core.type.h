#ifndef SPLRT_TYPE_H
#define SPLTY_TYPE_H

#include "core.h"

typedef void iohandler_templet_t(
   const char *fmt,
   persona_t *arr,
   int charidx
);

static void clearbuf(void);
static iohandler_templet_t io_out;
static iohandler_templet_t io_in;

#endif
