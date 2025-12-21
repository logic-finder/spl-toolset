#include "stack.adt.h"
#include "stack.adt.type.h"

extern stack_t *stack_create(void) {
   stack_t *ret;

   ret = malloc(sizeof *ret);
   if (!ret) raise_err("malloc failure");

   ret->siz = 0;
   ret->top = NULL;

   return ret;
}

static bool stack_empty(stack_t *stack) {
   return stack->siz == 0;
}

extern void stack_destroy(stack_t *stack) {
   while (stack_empty(stack))
      stack_pop(stack, NULL);
   free(stack);
}

extern void stack_push(stack_t *stack, int v) {
   node_t *n;

   n = malloc(sizeof *n);
   if (!n) raise_err("malloc failure");

   if (stack_empty(stack)) {
      stack->top = n;
      n->next = n->prev = NULL;
   }
   else {
      stack->top->next = n;
      n->prev = stack->top;
      n->next = NULL;
      stack->top = n;
   }

   n->v = v;
   stack->siz++;
}

extern void stack_pop(stack_t *stack, int *v) {
   node_t *n;

   if (stack_empty(stack))
      raise_err("attempt to pop from an empty stack");

   n = stack->top;
   if (v)
      *v = n->v;

   if (stack->siz == 1)
      stack->top = NULL;
   else {
      stack->top->prev->next = NULL;
      stack->top = stack->top->prev;
   }

   free(n);
   stack->siz--;
}
