#include "defs.h"
#include "util.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <math.h>
#include <stdio.h>

Color Color_RED = {255, 0, 0};
Color Color_BLUE = {0, 0, 255};
Color Color_GREEN = {0, 255, 0};
Color Color_BLACK = {0, 0, 0};
Color Color_WHITE = {255, 255, 255};
Color Color_BG = {25, 25, 35};
Color Color_CIRCLE = {100, 200, 255};

extern State state;

void set_draw_color(Color color) {
  SDL_SetRenderDrawColor(state.renderer, color.r, color.g, color.b, 255);
}

void draw_circle(Circle *c) {
  set_draw_color(c->color);
  
  int radius = (int)c->radius;
  int center_x = (int)round(c->xcenter);
  int center_y = (int)round(c->ycenter);
  
  // Draw filled circle by drawing horizontal lines
  for (int y = -radius; y <= radius; y++) {
    int width = (int)sqrt(radius * radius - y * y);
    for (int x = -width; x <= width; x++) {
      SDL_RenderDrawPoint(state.renderer, center_x + x, center_y + y);
    }
  }
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
  // Close font
  if (state.font) {
    TTF_CloseFont(state.font);
    state.font = NULL;
  }

  // Destroy renderer and window
  SDL_DestroyRenderer(state.renderer);
  SDL_DestroyWindow(state.window);

  // Quit SDL subsystems
  TTF_Quit();
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

  // Initialize SDL_ttf
  if (TTF_Init() < 0) {
    printf("Could not initialize SDL_ttf: %s\n", TTF_GetError());
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
  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == NULL) {
    printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  clear_screen();
  state.renderer = renderer;

  // Load font (try common system font paths)
  state.font =
      TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 16);
  if (!state.font) {
    state.font = TTF_OpenFont("/System/Library/Fonts/Arial.ttf", 16);
  }
  if (!state.font) {
    state.font = TTF_OpenFont("/usr/share/fonts/TTF/arial.ttf", 16);
  }
  if (!state.font) {
    printf("Could not load font: %s\n", TTF_GetError());
    return 1;
  }

  state.particles = NULL;
  state.particle_count = 0;
  return 0;
}

void draw_fps() {
  char fps_text[32];
  snprintf(fps_text, sizeof(fps_text), "FPS: %.0f", state.fps);

  // Create surface from text
  SDL_Color white = {255, 255, 255, 255};
  SDL_Surface *text_surface = TTF_RenderText_Solid(state.font, fps_text, white);
  if (!text_surface) {
    return;
  }

  // Create texture from surface
  SDL_Texture *text_texture =
      SDL_CreateTextureFromSurface(state.renderer, text_surface);
  if (!text_texture) {
    SDL_FreeSurface(text_surface);
    return;
  }

  // Get text dimensions
  int text_width = text_surface->w;
  int text_height = text_surface->h;
  SDL_FreeSurface(text_surface);

  // Position text in top-right corner
  SDL_Rect text_rect = {SCREEN_WIDTH - text_width - 10, 10, text_width,
                        text_height};

  // Draw background rectangle
  SDL_Rect bg_rect = {text_rect.x - 5, text_rect.y - 2, text_width + 10,
                      text_height + 4};
  set_draw_color(Color_BLACK);
  SDL_RenderFillRect(state.renderer, &bg_rect);

  // Render text
  SDL_RenderCopy(state.renderer, text_texture, NULL, &text_rect);

  // Cleanup
  SDL_DestroyTexture(text_texture);
}

void render() {
  for (int i = 0; i < state.particle_count; i++) {
    draw_circle(&state.particles[i]);
    //    draw_velocity_vector(&state.particles[i]);
  }
  draw_borders();
  draw_fps();
  present();
}
