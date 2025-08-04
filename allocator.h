#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "defs.h"

typedef struct Allocator {
  Node *pool;
  int *free_list;
  int capacity;
  int next_free;
  int allocated_count;
} Allocator;

int allocator_init(int capacity);
Node *allocator_alloc_node();
void allocator_free_node(Node *node);
void allocator_reset();
void allocator_cleanup();

#endif