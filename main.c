#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Screen dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const float framerate = 60.0;
const float dt = 1.0 / framerate;
const float g = 9.80;
const int num_circles = 10;

typedef struct Circle {
  float xcenter;
  float ycenter;
  float radius;
  float dx;
  float dy;
  float yvelocity;
} Circle;

// Define the structure for a linked list node
typedef struct Node {
  Circle object;
  struct Node *next;
} Node;

typedef struct State {
  SDL_Renderer *renderer;
  SDL_Window *window;
  Node *head;
} State;

struct SimulationState {
  double velocity;
  double position;
};

typedef struct Color {
  Uint8 r;
  Uint8 g;
  Uint8 b;
} Color;

const Color Color_RED = {255, 0, 0};
const Color Color_BLUE = {0, 0, 255};
const Color Color_GREEN = {0, 255, 0};
const Color Color_BLACK = {0, 0, 0};

State state;

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

void redraw() {
  SDL_RenderPresent(state.renderer);
  clear_screen();
}

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

int randInt(int min, int max) {
  return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

Circle create_circle() {
  Circle c = {
      .ycenter = (float)randInt(0, SCREEN_HEIGHT),
      .xcenter = (float)randInt(0, SCREEN_WIDTH),
      .radius = 10.0,
      .yvelocity = 0.0,
  };
  return c;
}

// Function to create a new node
Node *create_node() {
  Node *newNode = (Node *)malloc(sizeof(Node));
  if (!newNode) {
    printf("Memory allocation failed\n");
    exit(1);
  }
  newNode->object = create_circle();
  newNode->next = NULL;
  return newNode;
}

// Function to add a node to the end of the list
void add_node() {
  Node *newNode = create_node();
  if (state.head == NULL) {
    state.head = newNode;
  } else {
    Node *temp = state.head;
    while (temp->next != NULL) {
      temp = temp->next;
    }
    temp->next = newNode;
  }
}

void freelist() {
  Node *temp;
  while (state.head != NULL) {
    temp = state.head;
    state.head = state.head->next;
    free(temp);
  }
}

bool user_interrupt() {
  SDL_Event e;
  if (SDL_PollEvent(&e) != 0) {
    // User requests quit
    if (e.type == SDL_QUIT) {
      return true;
    }
  }
  return false;
}

void update_location(float dt) {
  float dv = dt * g;

  Node *temp = state.head;
  Node *prev = NULL;

  while (temp != NULL) {
    temp->object.yvelocity += dt * g;
    temp->object.ycenter += temp->object.yvelocity * dt;

    // TODO: take into account the radius.
    if (temp->object.ycenter - temp->object.radius < 0 ||
        temp->object.ycenter + temp->object.radius > SCREEN_HEIGHT ||
        temp->object.xcenter - temp->object.radius < 0 ||
        temp->object.xcenter + temp->object.radius > SCREEN_WIDTH) {
      Node *nodeToRemove = temp;

      if (prev == NULL) {
        printf("new head\n");
        printf("%f\n", temp->object.ycenter);
        state.head = temp->next;
        temp = temp->next;
      } else {
        prev->next = temp->next;
        temp = temp->next;
      }
      free(nodeToRemove);
    } else {
      prev = temp;
      temp = temp->next;
    }
  }
}

void render_circles() {
  Node *temp = state.head;
  while (temp != NULL) {
    redraw();
    draw_circle(&temp->object);
    temp = temp->next;
  }
}

void generate_circles() {
  for (int i = 0; i < num_circles; i++) {
    add_node();
  }
}

int main(int argc, char *args[]) {
  if (setup() > 0) {
    exit(-1);
  };

  set_draw_color(Color_RED);

  generate_circles();
  // runs until user exits
  uint64_t t0 = SDL_GetTicks();

  while (!user_interrupt()) {
    uint32_t now = SDL_GetTicks();
    float dt = (now - t0) / 1000.0f;

    update_location(dt);
    render_circles();
  }

  cleanup();
  freelist();
  return 0;
}
