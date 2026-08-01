#ifndef STACK_ADT_H
#define STACK_ADT_H

#include <stdbool.h>

typedef struct stack stack_t;

stack_t *stack_create(void);
void stack_destroy(stack_t *s);
void stack_push(stack_t *stack, int v);
void stack_pop(stack_t *stack, int *v);
bool stack_empty(stack_t *s);

#endif
