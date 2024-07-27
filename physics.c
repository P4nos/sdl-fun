#include "physics.h"
#include <SDL2/SDL_timer.h>

void calculate_location(Node *node) {
  Uint32 time = SDL_GetTicks();
  float dt = (time - node->object.lastupdated) / 1000.0f;
  //    float dv = dt * g;

  // don't let the circles move past the end of the screen height
  if (node->object.ycenter + node->object.radius > SCREEN_HEIGHT) {
    node->object.yvelocity = 0.0;
    node->object.dy += 0.0;
    node->object.ycenter = SCREEN_HEIGHT - node->object.radius;
  } else {
    node->object.yvelocity += dt * GRAVITY;
    node->object.dy += GRAVITY * dt * dt;
    node->object.ycenter += node->object.dy;
  }
}
