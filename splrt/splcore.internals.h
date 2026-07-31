#ifndef SPLRT_INTERNALS_H
#define SPLRT_INTERNALS_H

typedef void iohandler_templet_t(
   const char *fmt,
   persona_t *arr,
   int charidx
);

static void clearbuf(void);
static iohandler_templet_t io_out;
static iohandler_templet_t io_in;

#endif
