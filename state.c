#include "allocator.h"
#include "defs.h"
#include "physics.h"
#include "util.h"
#include <SDL2/SDL_timer.h>
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
    if (grid_x < 0)
      grid_x = 0;
    if (grid_x >= GRID_WIDTH)
      grid_x = GRID_WIDTH - 1;
    if (grid_y < 0)
      grid_y = 0;
    if (grid_y >= GRID_HEIGHT)
      grid_y = GRID_HEIGHT - 1;

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
  state.fps = 0.0f;
  state.last_fps_update = SDL_GetTicks();
  state.frame_count = 0;

  // Initialize settings
  state.settings.gravity = GRAVITY;
  state.settings.num_particles = MAX_SOURCE_PARTICLES;
  state.settings.initial_velocity_min = INITIAL_VELOCITY_MIN;
  state.settings.initial_velocity_max = INITIAL_VELOCITY_MAX;
  state.settings.show_settings = 1;
  state.settings.is_paused = 0;
  state.settings.show_velocity_vectors = 0;

  // Initialize particle source using constants
  state.source.x = SOURCE_X;
  state.source.y = SOURCE_Y;
  state.source.width = SOURCE_SIZE;
  state.source.height = SOURCE_SIZE;
  state.source.flow_rate = SOURCE_FLOW_RATE;
  state.source.velocity_magnitude = SOURCE_VELOCITY_MAGNITUDE;
  state.source.last_spawn_time = SDL_GetTicks();
  state.source.is_active = 1;
  state.source.particles_spawned = 0;
  state.source.emitter_side = EMITTER_RIGHT;
}

void update_particle_source() {
  if (!state.source.is_active)
    return;

  // Stop generating if we've reached the maximum source particles
  if (state.source.particles_spawned >= MAX_SOURCE_PARTICLES) {
    state.source.is_active = 0; // Deactivate source when limit reached
    return;
  }

  Uint32 current_time = SDL_GetTicks();
  float dt = (current_time - state.source.last_spawn_time) / 1000.0f;
  float spawn_interval = 1.0f / state.source.flow_rate;

  // Check if it's time to spawn a new particle
  if (dt >= spawn_interval && state.particle_count < MAX_SOURCE_PARTICLES) {
    float spawn_x, spawn_y;
    float velocity_x, velocity_y;

    // Calculate spawn position and velocity based on emitter side
    switch (state.source.emitter_side) {
    case EMITTER_LEFT:
      spawn_x = state.source.x;
      spawn_y =
          state.source.y + (float)rand_int_range(0, (int)state.source.height);
      velocity_x = -state.source.velocity_magnitude; // Leftward (negative)
      velocity_y = 0.0f;
      break;
    case EMITTER_RIGHT:
      spawn_x = state.source.x + state.source.width;
      spawn_y =
          state.source.y + (float)rand_int_range(0, (int)state.source.height);
      velocity_x = state.source.velocity_magnitude; // Rightward (positive)
      velocity_y = 0.0f;
      break;
    case EMITTER_TOP:
      spawn_x =
          state.source.x + (float)rand_int_range(0, (int)state.source.width);
      spawn_y = state.source.y;
      velocity_x = 0.0f;
      velocity_y = -state.source.velocity_magnitude; // Upward (negative)
      break;
    case EMITTER_BOTTOM:
      spawn_x =
          state.source.x + (float)rand_int_range(0, (int)state.source.width);
      spawn_y = state.source.y + state.source.height;
      velocity_x = 0.0f;
      velocity_y = state.source.velocity_magnitude; // Downward (positive)
      break;
    default:
      // Default to right emitter
      spawn_x = state.source.x + state.source.width;
      spawn_y =
          state.source.y + (float)rand_int_range(0, (int)state.source.height);
      velocity_x = state.source.velocity_magnitude; // Rightward (positive)
      velocity_y = 0.0f;
      break;
    }

    Circle new_particle = {.xcenter = spawn_x,
                           .ycenter = spawn_y,
                           .radius = PARTICLE_RADIUS,
                           .xvelocity = velocity_x,
                           .yvelocity = velocity_y,
                           .m = 20.0f,
                           .cor = 0.80f,
                           .dx = 0.0f,
                           .dy = 0.0f,
                           .color = USE_RANDOM_COLORS ? generate_random_color()
                                                      : Color_CIRCLE,
                           .id = state.particle_count,
                           .lastupdated = current_time};

    add_particle(new_particle);
    state.source.particles_spawned++;
    state.source.last_spawn_time = current_time;
  }
}

void update_fps() {
  state.frame_count++;
  Uint32 current_time = SDL_GetTicks();
  Uint32 elapsed = current_time - state.last_fps_update;

  // Update FPS every 500ms
  if (elapsed >= 500) {
    state.fps = (float)state.frame_count * 1000.0f / (float)elapsed;
    state.frame_count = 0;
    state.last_fps_update = current_time;
  }
}
