#include <stdlib.h>
#include "util.h"

int rand_int_range(int lower, int upper) {
  return rand() % (upper + 1 - lower) + lower;
}

Color generate_random_color() {
  Color random_color;
  random_color.r = rand_int_range(50, 255);  // Avoid very dark colors
  random_color.g = rand_int_range(50, 255);
  random_color.b = rand_int_range(50, 255);
  return random_color;
}
