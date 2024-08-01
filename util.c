#include "defs.h"
#include <stdio.h>

void print_circle_info(Circle c) {
  printf("id: %d\n", c.id);
  printf("xcenter: %f\n", c.xcenter);
  printf("ycenter: %f\n", c.ycenter);
  printf("radius: %f\n", c.radius);
  printf("dx: %f\n", c.dx);
  printf("dy: %f\n", c.dy);
  printf("yvelocity: %f\n", c.yvelocity);
  printf("xvelocity: %f\n", c.xvelocity);
  printf("lastupdated: %f\n", c.lastupdated);
  printf("m: %f\n", c.m);
}
