#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include "defs.h"
#include "state.h"

State state;

int main() {
  srand(time(NULL));

  if (setup() > 0) {
    exit(-1);
  };

  init_state();

  SDL_Event e;
  bool running = true;
  while (running) {
    // event loop
    while (SDL_PollEvent(&e) != 0) {
      switch (e.type) {
      case SDL_QUIT:
        running = false;
        break;
      case SDL_KEYDOWN:
        switch (e.key.keysym.sym) {
        case SDLK_j:
          update_state();
          break;
        case SDLK_r:
          reset_state();
          init_state();
          break;
        }
      }
    }

    clear_screen();
    // physics loop
    update_state();
    // render loop
    render();
  }

  cleanup();
  reset_state();
  return 0;
}
