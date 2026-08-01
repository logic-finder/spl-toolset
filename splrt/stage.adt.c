#include "stage.adt.h"
#include "stage.adt.internals.h"

extern stage_t *stage_create(size_t dpsz) {
   stage_t *ret;

   ret = safe_malloc(sizeof *ret);

   ret->cnt = 0;
   ret->dpsz = dpsz;
   ret->states = scalloc(dpsz, ESIZ(ret->states));  /* bool [] */
   ret->names = safe_malloc(dpsz * ESIZ(ret->names));  /* string [] */

   return ret;
}

extern void stage_destroy(stage_t *st) {
   cleanup_names(st);
   free(st->states);
   free(st);
}

extern bool stage_onstage(stage_t *st, size_t charidx) {
   return st->states[charidx];
}

extern void stage_enter(stage_t *st, size_t charidx) {
   if (stage_onstage(st, charidx))
      VERR(
         "attempt to Enter \"%s\", who is already onstage",
         stage_name(st, charidx)
      );
   st->states[charidx] = true;
   st->cnt++;
}

extern void stage_exit(stage_t *st, int charidx) {
   if (!stage_onstage(st, charidx))
      VERR(
         "attempt to Exit \"%s\", who is already offstage",
         stage_name(st, charidx)
      );
   st->states[charidx] = false;
   st->cnt--;
}

extern void stage_exeunt(stage_t *st) {
   for (size_t i = 0; i < st->dpsz; i++)
      if (stage_onstage(st, i))
         stage_exit(st, i);
}

// fixme: meaningless?
extern bool stage_aretheretwo(stage_t *st) {
   return st->cnt == 2;
}

extern size_t stage_whoareyou(stage_t *st, size_t me_idx) {
   if (st->cnt != 2)
      VERR(
         "the number of onstage characters not 2; currently %d",
         st->cnt
      );

   for (size_t i = 0; i < st->dpsz; i++) {
      if (i == me_idx)
         continue;
      if (stage_onstage(st, i))
         return i;
   }

   /* control never reaches here */
   return 0;
}

extern const char *stage_name(stage_t *st, size_t charidx) {
   return st->names[charidx];
}

extern size_t stage_cnt(stage_t *st) {
   return st->cnt;
}

extern void stage_setname(
   stage_t *st,
   size_t charidx,
   const char *name
) {
   size_t siz;
   char *buf;

   siz = strlen(name) + 1;  /* +1 for \0 */
   buf = safe_malloc(siz);
   memcpy(buf, name, siz);

   st->names[charidx] = buf;
}

static void cleanup_names(stage_t *st) {
   for (size_t i = 0; i < st->dpsz; i++)
      free(st->names[i]);
   free(st->names);
}
