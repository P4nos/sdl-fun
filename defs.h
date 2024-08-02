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
#define NUM_CIRCLES 160

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
