#include "physics.h"
#include "util.h"
#include "allocator.h"

extern State state;

// Function to add a particle to the array
void add_particle(Circle c) {
  int index = allocator_alloc_particle();
  if (index < 0) {
    printf("Memory allocation failed\n");
    exit(1);
  }

  state.particles[index] = c;
  state.particle_count++;
}

void reset_state() {
  allocator_reset();
  state.particle_count = 0;
}

void update_state() {
  for (int i = 0; i < state.particle_count; i++) {
    // calculate new location for object
    calculate_location(&state.particles[i]);
    // detect collisions
    handle_object_collisions(i);
    handle_border_collisions(&state.particles[i]);
  }
}

void init_state() {
  state.particles = allocator_get_pool();
  state.particle_count = 0;
  
  for (int i = 0; i < NUM_CIRCLES; i++) {
    Circle c = {.ycenter = (float)rand_int_range(0, SCREEN_HEIGHT / 2),
                .xcenter = (float)rand_int_range(0, SCREEN_WIDTH),
                .radius = 2.0,
                .yvelocity = (float)rand_int_range(-200, 200),
                .xvelocity = (float)rand_int_range(-200, 200),
                .m = 1.0,
                .dx = 0.0,
                .dy = 0.0,
                .color = get_rand_color(),
                .id = i};
    add_particle(c);
  }
}
