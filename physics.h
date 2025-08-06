#ifndef PHYSICS_H
#define PHYSICS_H

#include "defs.h"

void handle_grid_cell_collisions(int grid_x, int grid_y);
void handle_border_collisions(Circle *particle);
void calculate_location(Circle *particle);
void calculate_location_dt(Circle *particle, float dt);

#endif
