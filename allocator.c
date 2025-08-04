#include <stdlib.h>
#include <stdio.h>
#include "allocator.h"

static Allocator allocator;

int allocator_init(int capacity) {
  allocator.capacity = capacity;
  allocator.allocated_count = 0;
  allocator.next_free = 0;
  
  allocator.pool = (Node *)malloc(capacity * sizeof(Node));
  if (!allocator.pool) {
    printf("Failed to allocate memory pool\n");
    return -1;
  }
  
  allocator.free_list = (int *)malloc(capacity * sizeof(int));
  if (!allocator.free_list) {
    printf("Failed to allocate free list\n");
    free(allocator.pool);
    return -1;
  }
  
  for (int i = 0; i < capacity; i++) {
    allocator.free_list[i] = i;
  }
  
  return 0;
}

Node *allocator_alloc_node() {
  if (allocator.allocated_count >= allocator.capacity) {
    printf("Allocator pool exhausted\n");
    return NULL;
  }
  
  int index = allocator.free_list[allocator.next_free];
  allocator.next_free++;
  allocator.allocated_count++;
  
  Node *node = &allocator.pool[index];
  node->next = NULL;
  
  return node;
}

void allocator_free_node(Node *node) {
  if (!node || allocator.allocated_count == 0) {
    return;
  }
  
  int index = node - allocator.pool;
  if (index < 0 || index >= allocator.capacity) {
    printf("Invalid node pointer in allocator_free_node\n");
    return;
  }
  
  allocator.next_free--;
  allocator.free_list[allocator.next_free] = index;
  allocator.allocated_count--;
}

void allocator_reset() {
  allocator.allocated_count = 0;
  allocator.next_free = 0;
  
  for (int i = 0; i < allocator.capacity; i++) {
    allocator.free_list[i] = i;
  }
}

void allocator_cleanup() {
  if (allocator.pool) {
    free(allocator.pool);
    allocator.pool = NULL;
  }
  
  if (allocator.free_list) {
    free(allocator.free_list);
    allocator.free_list = NULL;
  }
  
  allocator.capacity = 0;
  allocator.allocated_count = 0;
  allocator.next_free = 0;
}