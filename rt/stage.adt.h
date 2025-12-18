#ifndef STAGE_ADT_H
#define STAGE_ADT_H

#include <stdbool.h>

typedef struct stage stage_t;

stage_t *stage_create(int siz);
void stage_destroy(stage_t *stage);
void stage_enter(stage_t *stage, int charidx);
void stage_exit(stage_t *stage, int charidx);
void stage_exeunt(stage_t *stage);
bool stage_onstage(stage_t *stage, int charidx);
bool stage_aretheretwo(stage_t *stage);
int stage_whoareyou(stage_t *stage, int me);
const char *stage_name(stage_t *stage, int idx);
int stage_cnt(stage_t *stage);
void stage_setname(
   stage_t *stage,
   int idx,
   const char *name
);

#endif
