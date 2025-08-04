#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "defs.h"

typedef struct Allocator {
  Circle *pool;
  int *free_list;
  int capacity;
  int next_free;
  int allocated_count;
} Allocator;

int allocator_init(int capacity);
int allocator_alloc_particle();
void allocator_free_particle(int index);
void allocator_reset();
void allocator_cleanup();
Circle *allocator_get_pool();

#endif