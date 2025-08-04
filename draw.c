#include "defs.h"
#include "util.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <math.h>

Color Color_RED = {255, 0, 0};
Color Color_BLUE = {0, 0, 255};
Color Color_GREEN = {0, 255, 0};
Color Color_BLACK = {0, 0, 0};
Color Color_BG = {252, 239, 150};

extern State state;

void set_draw_color(Color color) {
  SDL_SetRenderDrawColor(state.renderer, color.r, color.g, color.b, 255);
}

void draw_circle(Circle *c) {
  int32_t r = c->radius;
  int32_t x = -r;
  int32_t y = 0;
  int32_t err = 2 - 2 * c->radius;

  set_draw_color(c->color);

  do {
    SDL_RenderDrawPoint(state.renderer, round(c->xcenter - x),
                        round(c->ycenter + y));
    SDL_RenderDrawPoint(state.renderer, round(c->xcenter - y),
                        round(c->ycenter - x));
    SDL_RenderDrawPoint(state.renderer, round(c->xcenter + x),
                        round(c->ycenter - y));
    SDL_RenderDrawPoint(state.renderer, round(c->xcenter + y),
                        round(c->ycenter + x));
    r = err;
    if (r <= y)
      err += ++y * 2 + 1;
    if (r > x || err > y)
      err += ++x * 2 + 1;
  } while (x < 0);
}

Color get_rand_color() {
  return (Color){
      .r = rand_int_range(0, 255),
      .b = rand_int_range(0, 255),
      .g = rand_int_range(0, 255),
  };
}

void draw_velocity_vector(Circle *c) {
  float vector_len_px = 30.;
  float theta = atanf(c->yvelocity / c->xvelocity);

  // "default case, both x and y are positive"
  float dy = vector_len_px * sinf(theta);
  float dx = vector_len_px * cosf(theta);

  if (c->xvelocity < 0. && c->yvelocity >= 0.) {
    theta = atanf(c->xvelocity / c->yvelocity);
    dx = vector_len_px * sinf(theta);
    dy = vector_len_px * cosf(theta);
  }
  if (c->xvelocity < 0. && c->yvelocity <= 0.) {
    theta = atanf(c->xvelocity / c->yvelocity);
    dx = vector_len_px * cosf(theta);
    dy = vector_len_px * sinf(theta);
  }
  if (c->xvelocity > 0. && c->yvelocity <= 0.) {
    dx = vector_len_px * cosf(theta);
    dy = vector_len_px * sinf(theta);
  }

  SDL_RenderDrawLine(state.renderer, c->xcenter, c->ycenter,
                     c->xcenter + round(dx), c->ycenter + round(dy));
}

void cleanup() {
  // Destroy renderer and window
  SDL_DestroyRenderer(state.renderer);
  SDL_DestroyWindow(state.window);
  SDL_Quit();
}

void clear_screen() {
  set_draw_color(Color_BG);

  SDL_RenderClear(state.renderer);
}

void present() { SDL_RenderPresent(state.renderer); }

void draw_borders() {
  SDL_Rect borders[4];
  SDL_Rect top = {.x = 0, .y = 0, .w = SCREEN_WIDTH, .h = BORDER_WIDTH};
  SDL_Rect right = {.y = 0,
                    .x = SCREEN_WIDTH - BORDER_WIDTH,
                    .w = BORDER_WIDTH,
                    .h = SCREEN_HEIGHT};
  SDL_Rect bottom = {.x = 0,
                     .y = SCREEN_HEIGHT - BORDER_WIDTH,
                     .w = SCREEN_WIDTH,
                     .h = BORDER_WIDTH};
  SDL_Rect left = {.x = 0, .y = 0, .w = BORDER_WIDTH, .h = SCREEN_HEIGHT};

  borders[0] = top;
  borders[1] = right;
  borders[2] = bottom;
  borders[3] = left;

  set_draw_color(Color_BLACK);
  SDL_RenderFillRects(state.renderer, borders, 4);
}

int setup() {
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  // Create window
  SDL_Window *window = SDL_CreateWindow(
      "Simple particle engine", SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (window == NULL) {
    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  state.window = window;

  // Create renderer
  SDL_Renderer *renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
  if (renderer == NULL) {
    printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  clear_screen();
  state.renderer = renderer;

  state.particles = NULL;
  state.particle_count = 0;
  return 0;
}

void render() {
  for (int i = 0; i < state.particle_count; i++) {
    draw_circle(&state.particles[i]);
    //    draw_velocity_vector(&state.particles[i]);
  }
  draw_borders();
  present();
}
