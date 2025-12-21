#ifndef STACK_ADT_TYPE_H
#define STACK_ADT_TYPE_H

#include <stdlib.h>
#include <stdbool.h>
#include "core.h"
#include "stack.adt.h"

typedef struct node {
   struct node *prev, *next;
   int v;
} node_t;

struct stack {
   int siz;
   node_t *top;
};

static bool stack_empty(stack_t *stack);

#endif
