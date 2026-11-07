#include "stack.adt.h"
#include "stack.adt.internals.h"

extern stack_t *stack_create(void) {
   stack_t *ret;

   if (!(ret = malloc(sizeof *ret))) {
      return NULL;
   }

   ret->siz = 0;
   ret->top = NULL;

   return ret;
}

extern bool stack_empty(stack_t *s) {
   return !s->siz;
}

extern void stack_destroy(stack_t *s) {
   while (!stack_empty(s)) {
      (void) stack_pop(s, NULL);
   }
   free(s);
}

extern bool stack_push(stack_t *s, void *dat, size_t sz) {
   stack_node_t *n = NULL;
   void *ndat = NULL;

   if (!(n = malloc(sizeof *n))) {
      goto err;
   }
   if (!(ndat = malloc(sizeof sz))) {
      goto err;
   }

   // Copy data
   memcpy(ndat, dat, sz);
   n->dat = ndat;
   n->sz = sz;

   // Put the node into stack
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
   s->siz++;

   return true;

err:
   free(ndat);
   free(n);

   return false;
}

extern bool stack_pop(stack_t *stack, void *dest) {
   stack_node_t *n;

   if (stack_empty(stack)) {
      return false;
      // fixme: remove: fatal("attempt to pop from an empty stack");
   }

   n = stack->top;

   if (dest) {
      memcpy(dest, n->dat, n->sz);
   }

   if (stack->siz == 1) {
      stack->top = NULL;
   }
   else {
      stack->top->prev->next = NULL;
      stack->top = stack->top->prev;
   }

   free(n);
   stack->siz--;

   return true;
}
