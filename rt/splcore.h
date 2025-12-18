#ifndef SPLRT_H
#define SPLRT_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "stack.adt.h"
#include "stage.adt.h"

/**********
 * MACROS *
 **********/
#define Creset    "\033[0m"      // color reset
#define Cbred     "\033[0;91m"   // bright red

typedef int persona_t;
typedef stack_t *memory_t;
typedef void iohandler_t(persona_t *arr, int charidx);

persona_t *init_personae(int siz);
memory_t *init_memories(int siz);
void cleanup_memories(memory_t *arr, int siz);

int op_sqrt(int v);
int op_squr(int v);
int op_cube(int v);
int op_fact(int v);

iohandler_t io_inn;
iohandler_t io_inc;
iohandler_t io_outn;
iohandler_t io_outc;

/*****************
 * MISCELLANEOUS *
 *****************/
void raise_err(const char *msg, ...);
void assert_offstage(stage_t *st, int who);
void assert_onlytwo(stage_t *st);

#endif
