#ifndef STAGE_ADT_TYPE_H
#define STAGE_ADT_TYPE_H

#include <stdlib.h>
#include "core.h"
#include "stage.adt.h"

struct stage {
   int cnt;
   int siz;
   bool *state;
   const char **name;
};

#endif
