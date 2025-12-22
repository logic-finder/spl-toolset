#ifndef STAGE_ADT_TYPE_H
#define STAGE_ADT_TYPE_H

#include <stdlib.h>
#include "splcore.h"
#include "stage.adt.h"

struct stage {
   int cnt;
   int siz;
   bool *state;
   const char **name;
};

#endif
