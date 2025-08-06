#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#include "defs.h"
#include "state.h"
#include "allocator.h"

State state;

int main() {
  srand(time(NULL));
  
  // Set OpenMP thread count (use all available cores)
  omp_set_num_threads(omp_get_max_threads());

  if (setup() > 0) {
    exit(-1);
  };

  if (allocator_init(NUM_CIRCLES) < 0) {
    cleanup();
    exit(-1);
  }

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
          // Step simulation: reset timestamps and run one update
          {
            Uint32 current_time = SDL_GetTicks();
            for (int i = 0; i < state.particle_count; i++) {
              state.particles[i].lastupdated = current_time;
            }
            update_state();
          }
          break;
        case SDLK_r:
          reset_state();
          init_state();
          break;
        case SDLK_s:
          state.settings.show_settings = !state.settings.show_settings;
          break;
        case SDLK_SPACE:
          state.settings.is_paused = !state.settings.is_paused;
          if (!state.settings.is_paused) {
            // When resuming, reset all particle timestamps to avoid time jumps
            Uint32 current_time = SDL_GetTicks();
            for (int i = 0; i < state.particle_count; i++) {
              state.particles[i].lastupdated = current_time;
            }
          }
          break;
        case SDLK_v:
          state.settings.show_velocity_vectors = !state.settings.show_velocity_vectors;
          break;
        case SDLK_q:
          running = false;
          break;
        }
      }
    }

    clear_screen();
    
    // Only update physics if simulation is not paused
    if (!state.settings.is_paused) {
      // physics loop
      update_state();
      // update fps counter only when simulation is running
      update_fps();
    }
    // render loop
    render();
  }

  cleanup();
  reset_state();
  allocator_cleanup();
  return 0;
}
