#ifndef ARENA_ADT_H
#define ARENA_ADT_H

#include <stddef.h>

typedef struct arena arena_t;
typedef void *(mem_allocator_t)(size_t siz);

arena_t *arena_create(void);
void arena_cleanup(arena_t *arena);
void arena_malloc(size_t siz);

#endif
