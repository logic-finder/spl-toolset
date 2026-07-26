#include "fatal.h"
#include "fatal.internals.h"

void fatal(const char *msg) {
   ffmtwrt(stderr, "fatal: %s\n", msg);
   exit(EXIT_FAILURE);
}

void vfatal(const char *msg, ...) {
   va_list ap;
   int ret;

   va_start(ap, msg);
   sfputs(stderr, "fatal: ");
   ret = vfprintf(stderr, msg, ap);
   if (ret < 0) exit(EXIT_FAILURE);
   va_end(ap);
   sfputs(stderr, "\n");
   exit(EXIT_FAILURE);
}
