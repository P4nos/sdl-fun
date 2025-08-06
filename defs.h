#ifndef DEFS_H
#define DEFS_H

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
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
#define NUM_CIRCLES 5000

#define INITIAL_Y_MIN 0
#define INITIAL_Y_MAX (SCREEN_HEIGHT / 2)
#define INITIAL_X_MIN 0
#define INITIAL_X_MAX SCREEN_WIDTH
#define INITIAL_VELOCITY_MIN 0
#define INITIAL_VELOCITY_MAX 5

#define SETTINGS_PANEL_WIDTH 250
#define SETTINGS_PANEL_HEIGHT 450
#define SETTINGS_PANEL_MARGIN 10

#define GRID_CELL_SIZE 8
#define GRID_WIDTH (SCREEN_WIDTH / GRID_CELL_SIZE)
#define GRID_HEIGHT (SCREEN_HEIGHT / GRID_CELL_SIZE)
#define MAX_PARTICLES_PER_CELL 32

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

typedef struct GridCell {
  int particle_indices[MAX_PARTICLES_PER_CELL];
  int count;
} GridCell;

typedef struct Settings {
  float gravity;
  int num_particles;
  float initial_velocity_min;
  float initial_velocity_max;
  int show_settings;
  int is_paused;
  int show_velocity_vectors;
} Settings;

typedef struct State {
  SDL_Renderer *renderer;
  SDL_Window *window;
  Circle *particles;
  int particle_count;
  GridCell grid[GRID_HEIGHT][GRID_WIDTH];
  float fps;
  Uint32 last_fps_update;
  int frame_count;
  TTF_Font *font;
  Settings settings;
} State;

#endif
