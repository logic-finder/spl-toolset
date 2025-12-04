#include "global.h"
#include "wrapper.h"
#include "colorcode.h"

extern void err_template(teller_t *tell, const char *color, const char *msg) {
   ffmtwrt(stderr, "%s%s%s", color, msg, Creset);
   (*tell)();
   exit(EXIT_FAILURE);
}
