#include "stage.adt.h"
#include "stage.adt.type.h"

extern stage_t *stage_create(int siz) {
   stage_t *ret;

   ret = malloc(sizeof *ret);
   if (!ret) raise_err("malloc failure");

   ret->cnt = 0;
   ret->siz = siz;
   ret->state = calloc(siz, sizeof ret->state[0]);  /* array of bool */
   if (!ret->state) raise_err("calloc failure");
   ret->name = malloc(siz * sizeof ret->name[0]);

   return ret;
}

extern void stage_destroy(stage_t *stage) {
   free(stage);
}

extern bool stage_onstage(stage_t *stage, int charidx) {
   return stage->state[charidx];
}

extern void stage_enter(stage_t *stage, int charidx) {
   if (stage_onstage(stage, charidx))
      raise_err(
         "attempt to Enter '%s' already onstage",
         stage_name(stage, charidx));
   stage->state[charidx] = true;
   stage->cnt++;
}

extern void stage_exit(stage_t *stage, int charidx) {
   if (!stage_onstage(stage, charidx))
      raise_err(
         "attempt to Exit '%s' already offstage",
         stage_name(stage, charidx));
   stage->state[charidx] = false;
   stage->cnt--;
}

extern void stage_exeunt(stage_t *stage) {
   for (int idx = 0; idx < stage->siz; idx++)
      if (stage_onstage(stage, idx))
         stage_exit(stage, idx);
}

extern bool stage_aretheretwo(stage_t *stage) {
   return stage->cnt == 2;
}

extern int stage_whoareyou(stage_t *stage, int me) {
   // assumes there are only two onstage characters
   for (int idx = 0; idx < stage->siz; idx++) {
      if (idx == me)
         continue;
      if (stage_onstage(stage, idx))
         return idx;
   }
   // control never reaches here under the preposition
   return -1;
}

extern const char *stage_name(stage_t *stage, int idx) {
   return stage->name[idx];
}

extern int stage_cnt(stage_t *stage) {
   return stage->cnt;
}

extern void stage_setname(
   stage_t *stage,
   int idx,
   const char *name
) {
   stage->name[idx] = name;
}
