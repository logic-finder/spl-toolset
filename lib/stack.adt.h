#ifndef STACK_ADT_H
#define STACK_ADT_H

#include <stdbool.h>

typedef struct stack stack_t;

[[nodiscard]]
stack_t *stack_create(void);

void stack_destroy(stack_t *s);

[[nodiscard]]
bool stack_push(stack_t *s, void *dat, size_t sz);

[[nodiscard]]
bool stack_pop(stack_t *stack, void *dest);

bool stack_empty(stack_t *s);

#endif
