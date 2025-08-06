#include "physics.h"
#include "vector.h"
#include <SDL2/SDL_timer.h>
#include <math.h>

extern State state;

void resolve_collision(Circle *c1, Circle *c2) {
  // Calculate distance and separation first
  float dx = c1->xcenter - c2->xcenter;
  float dy = c1->ycenter - c2->ycenter;
  float dist = sqrtf(dx * dx + dy * dy);
  float overlap = (c1->radius + c2->radius) - dist;

  // Separate overlapping particles
  if (overlap > 0 && dist > 0) {
    float separation_x = (dx / dist) * (overlap * 0.5f);
    float separation_y = (dy / dist) * (overlap * 0.5f);

    c1->xcenter += separation_x;
    c1->ycenter += separation_y;
    c2->xcenter -= separation_x;
    c2->ycenter -= separation_y;
  }

  // follow the 7 steps https://www.vobarian.com/collisions/2dcollisions2.pdf

  // step 1: Find the unit tangent and unit normal vectors
  Vector n =
      create_vector(c1->xcenter - c2->xcenter, c1->ycenter - c2->ycenter, 0.);
  Vector u_n = unit_vector(n);
  Vector u_t = create_vector(-u_n.y, u_n.x, u_n.z);

  // step 2: Create velocity vectors (optional)
  Vector v1 = create_vector(c1->xvelocity, c1->yvelocity, 0.);
  Vector v2 = create_vector(c2->xvelocity, c2->yvelocity, 0.);
  // step 3: Resolve v1 and v2 into normal and tangential components
  float v1_n = dot_product(u_n, v1);
  float v1_t = dot_product(u_t, v1);
  float v2_n = dot_product(u_n, v2);
  float v2_t = dot_product(u_t, v2);

  // step 4: Find the new tangential velocities after the collision.
  // They do not change since there is no force between the two circles
  // in the tangential direction.
  float v1_t_prime = v1_t;
  float v2_t_prime = v2_t;

  // step 5: Find the new normal velocities using inelastic collision formula
  float combined_cor = sqrtf(c1->cor * c2->cor);
  float total_mass = c1->m + c2->m;

  float v1_n_prime =
      v1_n + (1.0f + combined_cor) * (v2_n - v1_n) * c2->m / total_mass;
  float v2_n_prime =
      v2_n + (1.0f + combined_cor) * (v1_n - v2_n) * c1->m / total_mass;

  Vector v1_n_prime_vec = scale_vector(v1_n_prime, u_n);
  Vector v1_t_prime_vec = scale_vector(v1_t_prime, u_t);
  Vector v2_n_prime_vec = scale_vector(v2_n_prime, u_n);
  Vector v2_t_prime_vec = scale_vector(v2_t_prime, u_t);

  Vector v1_prime_vec = add_vectors(v1_n_prime_vec, v1_t_prime_vec);

  Vector v2_prime_vec = add_vectors(v2_n_prime_vec, v2_t_prime_vec);

  c1->yvelocity = v1_prime_vec.y;
  c1->xvelocity = v1_prime_vec.x;

  c2->yvelocity = v2_prime_vec.y;
  c2->xvelocity = v2_prime_vec.x;
}

void handle_grid_cell_collisions(int grid_x, int grid_y) {
  GridCell *cell = &state.grid[grid_y][grid_x];

  // Check collisions within current cell
  for (int i = 0; i < cell->count; i++) {
    for (int j = i + 1; j < cell->count; j++) {
      int idx1 = cell->particle_indices[i];
      int idx2 = cell->particle_indices[j];

      Circle *p1 = &state.particles[idx1];
      Circle *p2 = &state.particles[idx2];

      float dx = p1->xcenter - p2->xcenter;
      float dy = p1->ycenter - p2->ycenter;
      float dist = eucledean_dist(dx, dy);

      if (dist <= p1->radius + p2->radius) {
        resolve_collision(p1, p2);
      }
    }
  }

  // Check collisions with adjacent cells (right and down to avoid duplicates)
  int adjacent_cells[2][2] = {{1, 0}, {0, 1}};

  for (int adj = 0; adj < 2; adj++) {
    int adj_x = grid_x + adjacent_cells[adj][0];
    int adj_y = grid_y + adjacent_cells[adj][1];

    if (adj_x < GRID_WIDTH && adj_y < GRID_HEIGHT) {
      GridCell *adj_cell = &state.grid[adj_y][adj_x];

      for (int i = 0; i < cell->count; i++) {
        for (int j = 0; j < adj_cell->count; j++) {
          int idx1 = cell->particle_indices[i];
          int idx2 = adj_cell->particle_indices[j];

          Circle *p1 = &state.particles[idx1];
          Circle *p2 = &state.particles[idx2];

          float dx = p1->xcenter - p2->xcenter;
          float dy = p1->ycenter - p2->ycenter;
          float dist = eucledean_dist(dx, dy);

          if (dist <= p1->radius + p2->radius) {
            resolve_collision(p1, p2);
          }
        }
      }
    }
  }
}

void handle_border_collisions(Circle *particle) {
  // object sides
  float left_point = particle->xcenter - particle->radius;
  float right_point = particle->xcenter + particle->radius;
  float top_point = particle->ycenter - particle->radius;
  float bottom_point = particle->ycenter + particle->radius;

  // walls
  float left_wall = BORDER_WIDTH;
  float right_wall = SCREEN_WIDTH - BORDER_WIDTH;
  float top_wall = BORDER_WIDTH;
  float bottom_wall = SCREEN_HEIGHT - BORDER_WIDTH;

  if (left_point < left_wall) {
    particle->xcenter = left_wall + particle->radius;
    particle->xvelocity *= -particle->cor;
  } else if (right_point > right_wall) {
    particle->xcenter = right_wall - particle->radius;
    particle->xvelocity *= -particle->cor;
  }

  if (top_point < top_wall) {
    particle->ycenter = top_wall + particle->radius;
    particle->yvelocity *= -particle->cor;
  } else if (bottom_point > bottom_wall) {
    particle->ycenter = bottom_wall - particle->radius;
    particle->yvelocity *= -particle->cor;
  }
}

void calculate_location(Circle *particle) {
  Uint32 time = SDL_GetTicks();
  float dt = (time - particle->lastupdated) / 1000.0f;

  // Apply gravity to y-velocity
  particle->yvelocity += GRAVITY * dt;

  particle->dy += particle->yvelocity * dt;
  particle->dx += particle->xvelocity * dt;

  particle->ycenter += particle->yvelocity * dt;
  particle->xcenter += particle->xvelocity * dt;

  particle->lastupdated = time;
}
