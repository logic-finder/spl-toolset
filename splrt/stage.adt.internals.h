#ifndef STAGE_ADT_INTERNALS_H
#define STAGE_ADT_INTERNALS_H

#include <stdlib.h>
#include <string.h>

#include "splrt.h"
#include "global.h"
#include "wrappers.h"

struct stage {
   size_t cnt, dpsz;
   bool *states;  /* bool [] */
   char **names;  /* string [] */
};

static void cleanup_names(stage_t *st);

#endif
