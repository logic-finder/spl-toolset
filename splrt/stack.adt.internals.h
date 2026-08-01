#ifndef STACK_ADT_INTERNALS_H
#define STACK_ADT_INTERNALS_H

#include <stdlib.h>
#include <stdbool.h>

#include "global.h"
#include "splcore.h"

typedef struct stack_node stack_node_t;

struct stack_node {
   stack_node_t *prev, *next;
   int v;
};

struct stack {
   int siz;
   stack_node_t *top;
};

#endif
