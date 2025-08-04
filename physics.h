#ifndef PHYSICS_H
#define PHYSICS_H

#include "defs.h"

void handle_object_collisions(int particle_index);
void handle_border_collisions(Circle *particle);
void calculate_location(Circle *particle);

#endif
