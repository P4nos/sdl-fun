#include "defs.h"
#include <SDL2/SDL.h>

Color Color_RED = {255, 0, 0};
Color Color_BLUE = {0, 0, 255};
Color Color_GREEN = {0, 255, 0};
Color Color_BLACK = {0, 0, 0};

extern State state;

void set_draw_color(Color color) {
  SDL_SetRenderDrawColor(state.renderer, color.r, color.g, color.b, 255);
}

void draw_circle(Circle *c) {

  set_draw_color(Color_RED);
  const int32_t diameter = (c->radius * 2);

  int32_t x = (c->radius - 1);
  int32_t y = 0;
  int32_t tx = 1;
  int32_t ty = 1;
  int32_t error = (tx - diameter);

  while (x >= y) {
    //  Each of the following renders an octant of the circle
    SDL_RenderDrawPoint(state.renderer, round(c->xcenter + x),
                        round(c->ycenter - y));
    SDL_RenderDrawPoint(state.renderer, round(c->xcenter + x),
                        round(c->ycenter + y));
    SDL_RenderDrawPoint(state.renderer, round(c->xcenter - x),
                        round(c->ycenter - y));
    SDL_RenderDrawPoint(state.renderer, round(c->xcenter - x),
                        round(c->ycenter + y));
    SDL_RenderDrawPoint(state.renderer, round(c->xcenter + y),
                        round(c->ycenter - x));
    SDL_RenderDrawPoint(state.renderer, round(c->xcenter + y),
                        round(c->ycenter + x));
    SDL_RenderDrawPoint(state.renderer, round(c->xcenter - y),
                        round(c->ycenter - x));
    SDL_RenderDrawPoint(state.renderer, round(c->xcenter - y),
                        round(c->ycenter + x));

    if (error <= 0) {
      ++y;
      error += ty;
      ty += 2;
    }

    if (error > 0) {
      --x;
      tx += 2;
      error += (tx - diameter);
    }
  }
}

void cleanup() {
  // Destroy renderer and window
  SDL_DestroyRenderer(state.renderer);
  SDL_DestroyWindow(state.window);
  SDL_Quit();
}

void clear_screen() {
  set_draw_color(Color_BLACK);

  SDL_RenderClear(state.renderer);
}

void present() { SDL_RenderPresent(state.renderer); }

int setup() {
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  // Create window
  SDL_Window *window = SDL_CreateWindow(
      "SDL2 Line Drawing", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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

  state.head = NULL;
  return 0;
}

void render() {
  Node *temp = state.head;
  while (temp != NULL) {
    draw_circle(&temp->object);
    temp = temp->next;
  }
  present();
}
