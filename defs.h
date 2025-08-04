#ifndef DEFS_H
#define DEFS_H

#include <SDL2/SDL_render.h>
#include <stdint.h>

#include "draw.h"

#ifdef DEBUG
#define DEBUG_LOGS 1
#else
#define DEBUG_LOGS 0
#endif

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define BORDER_WIDTH 5

#define GRAVITY 9.80 // gravity in pixels/s^2
#define NUM_CIRCLES 1600

typedef struct Circle {
  float xcenter;
  float ycenter;
  float radius;
  float dx;
  float dy;
  float yvelocity;
  float xvelocity;
  float lastupdated;
  float m; // mass
  Color color;
  int id;
} Circle;

typedef struct State {
  SDL_Renderer *renderer;
  SDL_Window *window;
  Circle *particles;
  int particle_count;
} State;

#endif
