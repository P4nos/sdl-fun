#include <stdlib.h>

int rand_int_range(int lower, int upper) {
  return rand() % (upper + 1 - lower) + lower;
}
