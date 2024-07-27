#include <SDL2/SDL_render.h>
#include <stdint.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define GRAVITY 9.80 // gravity in pixels/s^2
#define NUM_CIRCLES 150

typedef struct Circle {
  float xcenter;
  float ycenter;
  float radius;
  float dx;
  float dy;
  float yvelocity;
  float lastupdated;
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
  uint8_t r;
  uint8_t g;
  uint8_t b;
} Color;
