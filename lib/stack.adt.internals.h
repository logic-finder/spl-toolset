#ifndef STACK_ADT_INTERNALS_H
#define STACK_ADT_INTERNALS_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct stack_node stack_node_t;

struct stack_node {
   stack_node_t *prev, *next;
   void *dat;
   size_t sz;
};

struct stack {
   size_t siz;
   stack_node_t *top;
};

#endif
