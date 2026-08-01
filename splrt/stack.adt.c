#include "stack.adt.h"
#include "stack.adt.internals.h"

extern stack_t *stack_create(void) {
   stack_t *ret;

   ret = safe_malloc(sizeof *ret);

   ret->siz = 0;
   ret->top = NULL;

   return ret;
}

extern bool stack_empty(stack_t *s) {
   return !s->siz;
}

extern void stack_destroy(stack_t *s) {
   while (stack_empty(s))
      stack_pop(s, NULL);
   free(s);
}

extern void stack_push(stack_t *s, int v) {
   stack_node_t *n;

   n = safe_malloc(sizeof *n);

   if (stack_empty(s)) {
      s->top = n;
      n->next = n->prev = NULL;
   }
   else {
      s->top->next = n;
      n->prev = s->top;
      n->next = NULL;
      s->top = n;
   }

   n->v = v;
   s->siz++;
}

extern void stack_pop(stack_t *stack, int *v) {
   stack_node_t *n;

   if (stack_empty(stack))
      ERR("attempt to pop from an empty stack");

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
