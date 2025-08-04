#include "physics.h"
#include "util.h"
#include "allocator.h"
#include <omp.h>

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

void clear_grid() {
  #pragma omp parallel for collapse(2)
  for (int y = 0; y < GRID_HEIGHT; y++) {
    for (int x = 0; x < GRID_WIDTH; x++) {
      state.grid[y][x].count = 0;
    }
  }
}

void assign_particles_to_grid() {
  clear_grid();
  
  for (int i = 0; i < state.particle_count; i++) {
    Circle *p = &state.particles[i];
    int grid_x = (int)(p->xcenter / GRID_CELL_SIZE);
    int grid_y = (int)(p->ycenter / GRID_CELL_SIZE);
    
    // Clamp to grid bounds
    if (grid_x < 0) grid_x = 0;
    if (grid_x >= GRID_WIDTH) grid_x = GRID_WIDTH - 1;
    if (grid_y < 0) grid_y = 0;
    if (grid_y >= GRID_HEIGHT) grid_y = GRID_HEIGHT - 1;
    
    GridCell *cell = &state.grid[grid_y][grid_x];
    if (cell->count < MAX_PARTICLES_PER_CELL) {
      cell->particle_indices[cell->count] = i;
      cell->count++;
    }
  }
}

void update_state() {
  // Phase 1: Parallel position updates (no race conditions)
  #pragma omp parallel for
  for (int i = 0; i < state.particle_count; i++) {
    calculate_location(&state.particles[i]);
    handle_border_collisions(&state.particles[i]);
  }
  
  // Phase 2: Update spatial grid
  assign_particles_to_grid();
  
  // Phase 3: Spatial grid-based collision detection
  #pragma omp parallel for collapse(2)
  for (int y = 0; y < GRID_HEIGHT; y++) {
    for (int x = 0; x < GRID_WIDTH; x++) {
      handle_grid_cell_collisions(x, y);
    }
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
