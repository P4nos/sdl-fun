#include <SDL2/SDL.h>
#include <stdbool.h>

#include "draw.h"
#include "state.h"

State state;

int main() {
  if (setup() > 0) {
    exit(-1);
  };

  init_state();

  SDL_Event e;
  bool running = true;
  while (running) {

    clear_screen();
    // event loop
    while (SDL_PollEvent(&e) != 0) {
      switch (e.type) {
      case SDL_QUIT:
        running = false;
        break;
      }
    }
    // physics loop
    update_state();
    // render loop
    render();
  }

  cleanup();
  reset_state();
  return 0;
}
