#include <stdint.h>

typedef struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} Color;

int setup();
void render();
void set_draw_color(Color color);
void cleanup();
void clear_screen();
Color get_rand_color();
