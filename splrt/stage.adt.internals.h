#ifndef STAGE_ADT_INTERNALS_H
#define STAGE_ADT_INTERNALS_H

#include <stdlib.h>
#include "splcore.h"

struct stage {
   int cnt;
   int siz;
   bool *state;
   const char **name;
};

#endif
