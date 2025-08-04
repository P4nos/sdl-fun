#ifndef PHYSICS_H
#define PHYSICS_H

#include "defs.h"

void handle_object_collisions(Node *node);
void handle_border_collisions(Node *node);
void calculate_location(Node *node);

#endif
