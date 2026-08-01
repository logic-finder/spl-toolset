#ifndef STAGE_ADT_H
#define STAGE_ADT_H

#include <stddef.h>
#include <stdbool.h>

typedef struct stage stage_t;

stage_t *stage_create(size_t dpsz);
void stage_destroy(stage_t *st);
bool stage_onstage(stage_t *st, size_t charidx);
void stage_enter(stage_t *st, size_t charidx);
void stage_exit(stage_t *st, int charidx);
void stage_exeunt(stage_t *st);
bool stage_aretheretwo(stage_t *st);
size_t stage_whoareyou(stage_t *st, size_t me_idx);
const char *stage_name(stage_t *st, size_t charidx);
size_t stage_cnt(stage_t *st);
void stage_setname(
   stage_t *st,
   size_t charidx,
   const char *name
);

#endif
