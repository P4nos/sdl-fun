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
  // Skip if velocity is zero to avoid drawing zero-length vectors
  if (c->xvelocity == 0.0f && c->yvelocity == 0.0f) {
    return;
  }

  float vector_len_px = 30.0f;

  // Use atan2 for proper angle calculation in all quadrants
  float theta = atan2f(c->yvelocity, c->xvelocity);

  // Calculate vector endpoint
  float dx = vector_len_px * cosf(theta);
  float dy = vector_len_px * sinf(theta);

  // Set vector color (different from particles)
  SDL_SetRenderDrawColor(state.renderer, 255, 255, 0, 255); // Yellow

  SDL_RenderDrawLine(state.renderer, (int)c->xcenter, (int)c->ycenter,
                     (int)(c->xcenter + dx), (int)(c->ycenter + dy));
}

void draw_particle_source() {
  // Draw source rectangle
  SDL_Rect source_rect = {(int)state.source.x, (int)state.source.y,
                          (int)state.source.width, (int)state.source.height};

  if (state.source.is_active) {
    // Active source: green border with semi-transparent fill
    SDL_SetRenderDrawColor(state.renderer, 0, 255, 0, 255); // Green border
    SDL_RenderDrawRect(state.renderer, &source_rect);

    SDL_SetRenderDrawColor(state.renderer, 0, 255, 0,
                           64); // Semi-transparent green fill
    SDL_RenderFillRect(state.renderer, &source_rect);
  } else {
    // Inactive source: red border
    SDL_SetRenderDrawColor(state.renderer, 255, 0, 0, 255); // Red border
    SDL_RenderDrawRect(state.renderer, &source_rect);
  }
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

void draw_settings_panel() {
  if (!state.settings.show_settings || !state.font)
    return;

  // Calculate panel positio
  int panel_x = SCREEN_WIDTH - SETTINGS_PANEL_WIDTH;
  int panel_y = 10;

  // No background or border for settings panel

  SDL_Color white = {255, 255, 255, 255};
  char text[128];
  int y_offset = panel_y + 20;
  int line_height = 25;

  // FPS counter
  char fps_text[32];
  snprintf(fps_text, sizeof(fps_text), "FPS: %.0f", state.fps);
  SDL_Surface *fps_surface = TTF_RenderText_Solid(state.font, fps_text, white);
  if (fps_surface) {
    SDL_Texture *fps_texture =
        SDL_CreateTextureFromSurface(state.renderer, fps_surface);
    if (fps_texture) {
      SDL_Rect fps_rect = {panel_x + 10, y_offset, fps_surface->w,
                           fps_surface->h};
      SDL_RenderCopy(state.renderer, fps_texture, NULL, &fps_rect);
      SDL_DestroyTexture(fps_texture);
    }
    SDL_FreeSurface(fps_surface);
  }
  y_offset += line_height;

  // Particle count (particles_spawned)
  snprintf(text, sizeof(text), "Particles: %d", state.source.particles_spawned);
  SDL_Surface *particles_surface =
      TTF_RenderText_Solid(state.font, text, white);
  if (particles_surface) {
    SDL_Texture *particles_texture =
        SDL_CreateTextureFromSurface(state.renderer, particles_surface);
    if (particles_texture) {
      SDL_Rect particles_rect = {panel_x + 10, y_offset, particles_surface->w,
                                 particles_surface->h};
      SDL_RenderCopy(state.renderer, particles_texture, NULL, &particles_rect);
      SDL_DestroyTexture(particles_texture);
    }
    SDL_FreeSurface(particles_surface);
  }
  y_offset += line_height * 2;

  // Simulation status
  const char *status_text = state.settings.is_paused ? "PAUSED" : "RUNNING";
  snprintf(text, sizeof(text), "Status: %s", status_text);
  SDL_Surface *status_surface = TTF_RenderText_Solid(state.font, text, white);
  if (status_surface) {
    SDL_Texture *status_texture =
        SDL_CreateTextureFromSurface(state.renderer, status_surface);
    if (status_texture) {
      SDL_Rect status_rect = {panel_x + 10, y_offset, status_surface->w,
                              status_surface->h};
      SDL_RenderCopy(state.renderer, status_texture, NULL, &status_rect);
      SDL_DestroyTexture(status_texture);
    }
    SDL_FreeSurface(status_surface);
  }
  y_offset += line_height * 2;

  // Control instructions
  SDL_Surface *controls_surface =
      TTF_RenderText_Solid(state.font, "Controls:", white);
  if (controls_surface) {
    SDL_Texture *controls_texture =
        SDL_CreateTextureFromSurface(state.renderer, controls_surface);
    if (controls_texture) {
      SDL_Rect controls_rect = {panel_x + 10, y_offset, controls_surface->w,
                                controls_surface->h};
      SDL_RenderCopy(state.renderer, controls_texture, NULL, &controls_rect);
      SDL_DestroyTexture(controls_texture);
    }
    SDL_FreeSurface(controls_surface);
  }
  y_offset += line_height;

  SDL_Surface *space_surface =
      TTF_RenderText_Solid(state.font, "SPACE - Pause/Resume", white);
  if (space_surface) {
    SDL_Texture *space_texture =
        SDL_CreateTextureFromSurface(state.renderer, space_surface);
    if (space_texture) {
      SDL_Rect space_rect = {panel_x + 10, y_offset, space_surface->w,
                             space_surface->h};
      SDL_RenderCopy(state.renderer, space_texture, NULL, &space_rect);
      SDL_DestroyTexture(space_texture);
    }
    SDL_FreeSurface(space_surface);
  }
  y_offset += line_height;

  SDL_Surface *step_surface =
      TTF_RenderText_Solid(state.font, "J - Step simulation", white);
  if (step_surface) {
    SDL_Texture *step_texture =
        SDL_CreateTextureFromSurface(state.renderer, step_surface);
    if (step_texture) {
      SDL_Rect step_rect = {panel_x + 10, y_offset, step_surface->w,
                            step_surface->h};
      SDL_RenderCopy(state.renderer, step_texture, NULL, &step_rect);
      SDL_DestroyTexture(step_texture);
    }
    SDL_FreeSurface(step_surface);
  }
  y_offset += line_height;

  SDL_Surface *reset_surface =
      TTF_RenderText_Solid(state.font, "R - Reset simulation", white);
  if (reset_surface) {
    SDL_Texture *reset_texture =
        SDL_CreateTextureFromSurface(state.renderer, reset_surface);
    if (reset_texture) {
      SDL_Rect reset_rect = {panel_x + 10, y_offset, reset_surface->w,
                             reset_surface->h};
      SDL_RenderCopy(state.renderer, reset_texture, NULL, &reset_rect);
      SDL_DestroyTexture(reset_texture);
    }
    SDL_FreeSurface(reset_surface);
  }
  y_offset += line_height;

  SDL_Surface *vectors_surface =
      TTF_RenderText_Solid(state.font, "V - Toggle vectors", white);
  if (vectors_surface) {
    SDL_Texture *vectors_texture =
        SDL_CreateTextureFromSurface(state.renderer, vectors_surface);
    if (vectors_texture) {
      SDL_Rect vectors_rect = {panel_x + 10, y_offset, vectors_surface->w,
                               vectors_surface->h};
      SDL_RenderCopy(state.renderer, vectors_texture, NULL, &vectors_rect);
      SDL_DestroyTexture(vectors_texture);
    }
    SDL_FreeSurface(vectors_surface);
  }
  y_offset += line_height;

  SDL_Surface *settings_surface =
      TTF_RenderText_Solid(state.font, "S - Toggle settings", white);
  if (settings_surface) {
    SDL_Texture *settings_texture =
        SDL_CreateTextureFromSurface(state.renderer, settings_surface);
    if (settings_texture) {
      SDL_Rect settings_rect = {panel_x + 10, y_offset, settings_surface->w,
                                settings_surface->h};
      SDL_RenderCopy(state.renderer, settings_texture, NULL, &settings_rect);
      SDL_DestroyTexture(settings_texture);
    }
    SDL_FreeSurface(settings_surface);
  }
  y_offset += line_height;

  SDL_Surface *quit_surface =
      TTF_RenderText_Solid(state.font, "Q - Quit", white);
  if (quit_surface) {
    SDL_Texture *quit_texture =
        SDL_CreateTextureFromSurface(state.renderer, quit_surface);
    if (quit_texture) {
      SDL_Rect quit_rect = {panel_x + 10, y_offset, quit_surface->w,
                            quit_surface->h};
      SDL_RenderCopy(state.renderer, quit_texture, NULL, &quit_rect);
      SDL_DestroyTexture(quit_texture);
    }
    SDL_FreeSurface(quit_surface);
  }
}

void render() {
  for (int i = 0; i < state.particle_count; i++) {
    draw_circle(&state.particles[i]);
    if (state.settings.show_velocity_vectors) {
      draw_velocity_vector(&state.particles[i]);
    }
  }
  draw_borders();
  // draw_particle_source();
  draw_settings_panel();
  present();
}
