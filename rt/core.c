#include <stdio.h>
#include <stdlib.h>
#include "core.h"
#include "core.type.h"

extern persona_t *init_personae(int siz) {
   persona_t *ret;

   ret = malloc(siz * sizeof ret[0]);  /* array of int */
   if (!ret) raise_err("malloc failure");

   return ret;
}

extern memory_t *init_memories(int siz) {
   memory_t *ret;

   ret = malloc(siz * sizeof ret[0]);  /* array of stack */
   if (!ret) raise_err("malloc failure");
   for (int i = 0; i < siz; i++)
      ret[i] = stack_create();

   return ret;
}

extern void cleanup_memories(memory_t *arr, int siz) {
   for (int i = 0; i < siz; i++)
      free(arr[i]);
   free(arr);
}

extern int op_sqrt(int v) {
   return sqrt(v);
}

extern int op_squr(int v) {
   return v * v;
}

extern int op_cube(int v) {
   return v * v * v;
}

extern int op_fact(int v) {
   int r;

   for (r = 1; v >= 1; v--)
      r *= v;

   return r;
}

static void clearbuf(void) {
   int ch;
   while ((ch = getchar()) && ch != EOF)
      /* empty */ ;
   if (ferror(stdin))
      raise_err("getchar error");
}

static void io_in(const char *fmt, persona_t *arr, int charidx) {
   int ret = scanf(fmt, &arr[charidx]);
   if (ret != 1)
      raise_err("scanf error");
   clearbuf();
}

extern void io_inn(persona_t *arr, int charidx) {
   io_in("%d", arr, charidx);
}

extern void io_inc(persona_t *arr, int charidx) {
   io_in("%c", arr, charidx);
}

static void io_out(const char *fmt, persona_t *arr, int charidx) {
   int ret = printf(fmt, arr[charidx]);
   if (ret < 1)
      raise_err("printf error");
}

extern void io_outn(persona_t *arr, int charidx) {
   io_out("%d", arr, charidx);
}

extern void io_outc(persona_t *arr, int charidx) {
   io_out("%c", arr, charidx);
}

extern void raise_err(const char *msg, ...) {
   va_list ap;

   fputs(Cbred "<runtime error> " Creset, stderr);
   va_start(ap, msg);
   vfprintf(stderr, msg, ap);
   va_end(ap);
   fputc('\n', stderr);
   exit(EXIT_FAILURE);
}

extern void assert_offstage(stage_t *st, int who) {
   if (stage_onstage(st, who))
      return;
   raise_err(
      "a Line by offstage character %s",
      stage_name(st, who)
   );
}

extern void assert_onlytwo(stage_t *st) {
   if (stage_aretheretwo(st))
      return;
   raise_err(
      "the number of onstage characters not 2; currently %d",
      stage_cnt(st)
   );
}
