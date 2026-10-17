#ifndef ARENA_ADT_INTERNALS_H
#define ARENA_ADT_INTERNALS_H

#include "list.adt.h"
#include "arena.adt.h"

#define ARENA_SIZ_DEFAULT 8192

typedef struct {
   size_t cap;
   size_t pos;
} arena_region_t;

struct arena {
   list_t *regions;
};

#endif
