#include <stdlib.h>
#include <stdio.h>
#include "allocator.h"

static Allocator allocator;

int allocator_init(int capacity) {
  allocator.capacity = capacity;
  allocator.allocated_count = 0;
  allocator.next_free = 0;
  
  allocator.pool = (Circle *)malloc(capacity * sizeof(Circle));
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

int allocator_alloc_particle() {
  if (allocator.allocated_count >= allocator.capacity) {
    printf("Allocator pool exhausted\n");
    return -1;
  }
  
  int index = allocator.free_list[allocator.next_free];
  allocator.next_free++;
  allocator.allocated_count++;
  
  return index;
}

void allocator_free_particle(int index) {
  if (index < 0 || index >= allocator.capacity || allocator.allocated_count == 0) {
    printf("Invalid particle index in allocator_free_particle\n");
    return;
  }
  
  allocator.next_free--;
  allocator.free_list[allocator.next_free] = index;
  allocator.allocated_count--;
}

Circle *allocator_get_pool() {
  return allocator.pool;
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