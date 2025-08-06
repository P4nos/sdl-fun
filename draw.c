#include "defs.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

SDL_Texture *create_circle_texture(int radius) {
  int diameter = radius * 2;

  // Create a texture with per-pixel alpha
  SDL_Texture *texture =
      SDL_CreateTexture(state.renderer, SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_TARGET, diameter, diameter);
  if (!texture) {
    return NULL;
  }

  // Set blend mode for alpha blending
  SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

  // Set the texture as render target
  SDL_Texture *original_target = SDL_GetRenderTarget(state.renderer);
  SDL_SetRenderTarget(state.renderer, texture);

  // Clear with transparent background
  SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 0);
  SDL_RenderClear(state.renderer);

  // Draw white circle (color will be modulated when rendering)
  SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);

  // Draw filled circle by drawing horizontal lines
  for (int y = -radius; y < radius; y++) {
    int width = (int)sqrt(radius * radius - y * y);
    for (int x = -width; x < width; x++) {
      SDL_RenderDrawPoint(state.renderer, radius + x, radius + y);
    }
  }

  // Restore original render target
  SDL_SetRenderTarget(state.renderer, original_target);

  return texture;
}

int init_batch_rendering() {
  // Allocate vertex and index arrays for batched rendering
  // Each particle needs 4 vertices (quad) and 6 indices (2 triangles)
  state.max_vertices = MAX_SOURCE_PARTICLES * 4;
  state.max_indices = MAX_SOURCE_PARTICLES * 6;

  state.vertices = malloc(state.max_vertices * sizeof(SDL_Vertex));
  state.indices = malloc(state.max_indices * sizeof(int));

  if (!state.vertices || !state.indices) {
    printf("Failed to allocate batch rendering arrays\n");
    return -1;
  }

  return 0;
}

void cleanup_batch_rendering() {
  if (state.vertices) {
    free(state.vertices);
    state.vertices = NULL;
  }
  if (state.indices) {
    free(state.indices);
    state.indices = NULL;
  }
}

SDL_Texture *create_text_texture(const char *text, SDL_Color color) {
  if (!state.font)
    return NULL;

  SDL_Surface *surface = TTF_RenderText_Solid(state.font, text, color);
  if (!surface)
    return NULL;

  SDL_Texture *texture = SDL_CreateTextureFromSurface(state.renderer, surface);
  SDL_FreeSurface(surface);

  return texture;
}

int init_ui_cache() {
  SDL_Color white = {255, 255, 255, 255};

  // Initialize all pointers to NULL
  memset(&state.ui_cache, 0, sizeof(UICache));

  // Create static UI textures
  state.ui_cache.controls_label = create_text_texture("Controls:", white);
  state.ui_cache.space_help =
      create_text_texture("SPACE - Pause/Resume", white);
  state.ui_cache.step_help = create_text_texture("J - Step simulation", white);
  state.ui_cache.reset_help =
      create_text_texture("R - Reset simulation", white);
  state.ui_cache.vectors_help =
      create_text_texture("V - Toggle vectors", white);
  state.ui_cache.settings_help =
      create_text_texture("S - Toggle settings", white);
  state.ui_cache.quit_help = create_text_texture("Q - Quit", white);

  // Initialize dynamic cache values to invalid states
  state.ui_cache.last_fps = -1.0f;
  state.ui_cache.last_particle_count = -1;
  state.ui_cache.last_paused_state = -1;

  return 0;
}

void cleanup_ui_cache() {
  // Clean up static textures
  if (state.ui_cache.controls_label) {
    SDL_DestroyTexture(state.ui_cache.controls_label);
  }
  if (state.ui_cache.space_help) {
    SDL_DestroyTexture(state.ui_cache.space_help);
  }
  if (state.ui_cache.step_help) {
    SDL_DestroyTexture(state.ui_cache.step_help);
  }
  if (state.ui_cache.reset_help) {
    SDL_DestroyTexture(state.ui_cache.reset_help);
  }
  if (state.ui_cache.vectors_help) {
    SDL_DestroyTexture(state.ui_cache.vectors_help);
  }
  if (state.ui_cache.settings_help) {
    SDL_DestroyTexture(state.ui_cache.settings_help);
  }
  if (state.ui_cache.quit_help) {
    SDL_DestroyTexture(state.ui_cache.quit_help);
  }

  // Clean up dynamic textures
  if (state.ui_cache.fps_texture) {
    SDL_DestroyTexture(state.ui_cache.fps_texture);
  }
  if (state.ui_cache.particles_texture) {
    SDL_DestroyTexture(state.ui_cache.particles_texture);
  }
  if (state.ui_cache.status_texture) {
    SDL_DestroyTexture(state.ui_cache.status_texture);
  }

  // Clear the cache
  memset(&state.ui_cache, 0, sizeof(UICache));
}

void add_particle_to_batch(Circle *c, int particle_idx) {
  if (particle_idx >= MAX_SOURCE_PARTICLES)
    return;

  int radius = (int)c->radius;
  float center_x = c->xcenter;
  float center_y = c->ycenter;

  // Calculate quad vertices
  float left = center_x - radius;
  float right = center_x + radius;
  float top = center_y - radius;
  float bottom = center_y + radius;

  int vertex_offset = particle_idx * 4;
  int index_offset = particle_idx * 6;

  // Set vertex positions and texture coordinates
  // Top-left
  state.vertices[vertex_offset + 0].position.x = left;
  state.vertices[vertex_offset + 0].position.y = top;
  state.vertices[vertex_offset + 0].tex_coord.x = 0.0f;
  state.vertices[vertex_offset + 0].tex_coord.y = 0.0f;
  state.vertices[vertex_offset + 0].color.r = c->color.r;
  state.vertices[vertex_offset + 0].color.g = c->color.g;
  state.vertices[vertex_offset + 0].color.b = c->color.b;
  state.vertices[vertex_offset + 0].color.a = 255;

  // Top-right
  state.vertices[vertex_offset + 1].position.x = right;
  state.vertices[vertex_offset + 1].position.y = top;
  state.vertices[vertex_offset + 1].tex_coord.x = 1.0f;
  state.vertices[vertex_offset + 1].tex_coord.y = 0.0f;
  state.vertices[vertex_offset + 1].color.r = c->color.r;
  state.vertices[vertex_offset + 1].color.g = c->color.g;
  state.vertices[vertex_offset + 1].color.b = c->color.b;
  state.vertices[vertex_offset + 1].color.a = 255;

  // Bottom-left
  state.vertices[vertex_offset + 2].position.x = left;
  state.vertices[vertex_offset + 2].position.y = bottom;
  state.vertices[vertex_offset + 2].tex_coord.x = 0.0f;
  state.vertices[vertex_offset + 2].tex_coord.y = 1.0f;
  state.vertices[vertex_offset + 2].color.r = c->color.r;
  state.vertices[vertex_offset + 2].color.g = c->color.g;
  state.vertices[vertex_offset + 2].color.b = c->color.b;
  state.vertices[vertex_offset + 2].color.a = 255;

  // Bottom-right
  state.vertices[vertex_offset + 3].position.x = right;
  state.vertices[vertex_offset + 3].position.y = bottom;
  state.vertices[vertex_offset + 3].tex_coord.x = 1.0f;
  state.vertices[vertex_offset + 3].tex_coord.y = 1.0f;
  state.vertices[vertex_offset + 3].color.r = c->color.r;
  state.vertices[vertex_offset + 3].color.g = c->color.g;
  state.vertices[vertex_offset + 3].color.b = c->color.b;
  state.vertices[vertex_offset + 3].color.a = 255;

  // Set indices for two triangles (quad)
  int base_vertex = vertex_offset;
  state.indices[index_offset + 0] = base_vertex + 0; // Top-left
  state.indices[index_offset + 1] = base_vertex + 1; // Top-right
  state.indices[index_offset + 2] = base_vertex + 2; // Bottom-left
  state.indices[index_offset + 3] = base_vertex + 1; // Top-right
  state.indices[index_offset + 4] = base_vertex + 3; // Bottom-right
  state.indices[index_offset + 5] = base_vertex + 2; // Bottom-left
}

void render_particles_batched() {
  if (!state.circle_texture || !state.vertices || !state.indices ||
      state.particle_count == 0) {
    return;
  }

  // Build vertex array for all particles
  for (int i = 0; i < state.particle_count; i++) {
    add_particle_to_batch(&state.particles[i], i);
  }

  // Render all particles in one call
  SDL_RenderGeometry(state.renderer, state.circle_texture, state.vertices,
                     state.particle_count * 4, state.indices,
                     state.particle_count * 6);
}

void draw_circle(Circle *c) {
  if (!state.circle_texture) {
    return;
  }

  int radius = (int)c->radius;
  int center_x = (int)round(c->xcenter);
  int center_y = (int)round(c->ycenter);

  // Set color modulation for the texture
  SDL_SetTextureColorMod(state.circle_texture, c->color.r, c->color.g,
                         c->color.b);

  // Calculate destination rectangle
  SDL_Rect dest_rect = {center_x - radius, center_y - radius, radius * 2,
                        radius * 2};

  // Render the texture
  SDL_RenderCopy(state.renderer, state.circle_texture, NULL, &dest_rect);
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
  // Cleanup UI cache
  cleanup_ui_cache();

  // Cleanup batch rendering
  cleanup_batch_rendering();

  // Destroy circle texture
  if (state.circle_texture) {
    SDL_DestroyTexture(state.circle_texture);
    state.circle_texture = NULL;
  }

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

  // Create circle texture (using average particle radius)
  int default_radius = 2; // Matches typical particle size
  state.circle_texture = create_circle_texture(default_radius);
  state.circle_texture_size = default_radius * 2;
  if (!state.circle_texture) {
    printf("Warning: Could not create circle texture\n");
  }

  // Initialize batch rendering
  if (init_batch_rendering() < 0) {
    printf("Warning: Could not initialize batch rendering\n");
  }

  // Initialize UI cache
  if (init_ui_cache() < 0) {
    printf("Warning: Could not initialize UI cache\n");
  }

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

void update_dynamic_ui_textures() {
  SDL_Color white = {255, 255, 255, 255};
  char text[128];

  // Update FPS texture if changed
  if (state.fps != state.ui_cache.last_fps) {
    if (state.ui_cache.fps_texture) {
      SDL_DestroyTexture(state.ui_cache.fps_texture);
    }
    snprintf(text, sizeof(text), "FPS: %.0f", state.fps);
    state.ui_cache.fps_texture = create_text_texture(text, white);
    state.ui_cache.last_fps = state.fps;
  }

  // Update particle count texture if changed
  if (state.source.particles_spawned != state.ui_cache.last_particle_count) {
    if (state.ui_cache.particles_texture) {
      SDL_DestroyTexture(state.ui_cache.particles_texture);
    }
    snprintf(text, sizeof(text), "Particles: %d",
             state.source.particles_spawned);
    state.ui_cache.particles_texture = create_text_texture(text, white);
    state.ui_cache.last_particle_count = state.source.particles_spawned;
  }

  // Update status texture if changed
  int current_paused = state.settings.is_paused ? 1 : 0;
  if (current_paused != state.ui_cache.last_paused_state) {
    if (state.ui_cache.status_texture) {
      SDL_DestroyTexture(state.ui_cache.status_texture);
    }
    const char *status_text = state.settings.is_paused ? "PAUSED" : "RUNNING";
    snprintf(text, sizeof(text), "Status: %s", status_text);
    state.ui_cache.status_texture = create_text_texture(text, white);
    state.ui_cache.last_paused_state = current_paused;
  }
}

void draw_cached_texture(SDL_Texture *texture, int x, int y) {
  if (!texture)
    return;

  int w, h;
  SDL_QueryTexture(texture, NULL, NULL, &w, &h);
  SDL_Rect rect = {x, y, w, h};
  SDL_RenderCopy(state.renderer, texture, NULL, &rect);
}

void draw_settings_panel() {
  if (!state.settings.show_settings || !state.font)
    return;

  // Calculate panel position
  int panel_x = SCREEN_WIDTH - SETTINGS_PANEL_WIDTH;
  int panel_y = 10;
  int y_offset = panel_y + 20;
  int line_height = 25;
  int text_x = panel_x + 10;

  // Update dynamic textures only when values change
  update_dynamic_ui_textures();

  // Draw dynamic content using cached textures
  draw_cached_texture(state.ui_cache.fps_texture, text_x, y_offset);
  y_offset += line_height;

  draw_cached_texture(state.ui_cache.particles_texture, text_x, y_offset);
  y_offset += line_height * 2;

  draw_cached_texture(state.ui_cache.status_texture, text_x, y_offset);
  y_offset += line_height * 2;

  // Draw static content using cached textures
  draw_cached_texture(state.ui_cache.controls_label, text_x, y_offset);
  y_offset += line_height;

  draw_cached_texture(state.ui_cache.space_help, text_x, y_offset);
  y_offset += line_height;

  draw_cached_texture(state.ui_cache.step_help, text_x, y_offset);
  y_offset += line_height;

  draw_cached_texture(state.ui_cache.reset_help, text_x, y_offset);
  y_offset += line_height;

  draw_cached_texture(state.ui_cache.vectors_help, text_x, y_offset);
  y_offset += line_height;

  draw_cached_texture(state.ui_cache.settings_help, text_x, y_offset);
  y_offset += line_height;

  draw_cached_texture(state.ui_cache.quit_help, text_x, y_offset);
}

void render() {
  // Render all particles in a single batched call
  render_particles_batched();

  // Render velocity vectors if enabled
  if (state.settings.show_velocity_vectors) {
    for (int i = 0; i < state.particle_count; i++) {
      draw_velocity_vector(&state.particles[i]);
    }
  }

  draw_borders();
  // draw_particle_source();
  draw_settings_panel();
  present();
}
