#include "global.h"
#include "wrapper.h"
#include "colorcode.h"

extern void err_template(
   teller_t *tell,
   const char * restrict color,
   const char * restrict msg
) {
   ffmtwrt(stderr, "%s%s%s", color, msg, Creset);
   (*tell)();
   exit(EXIT_FAILURE);
}
