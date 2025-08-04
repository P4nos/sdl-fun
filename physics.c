#include "physics.h"
#include "vector.h"
#include <SDL2/SDL_timer.h>

extern State state;

int is_same_particle(int indexa, int indexb) {
  return indexa == indexb ? 1 : 0;
}

void resolve_collision(Circle *c1, Circle *c2) {
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

  // step 5: Find the new normal velocities
  float v1_n_prime =
      (v1_n * (c1->m - c2->m) + 2 * c2->m * v2_n) / (c1->m + c2->m);
  float v2_n_prime =
      (v2_n * (c2->m - c1->m) + 2 * c1->m * v1_n) / (c1->m + c2->m);

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

void handle_object_collisions(int particle_index) {
  Circle *particle = &state.particles[particle_index];
  
  for (int i = 0; i < state.particle_count; i++) {
    // don't check collisions with itself
    if (!is_same_particle(particle_index, i)) {
      Circle *other = &state.particles[i];
      
      // check distance between object centers
      float dy = particle->ycenter - other->ycenter;
      float dx = particle->xcenter - other->xcenter;
      float dist = eucledean_dist(dx, dy);

      // detect collision
      if (dist <= particle->radius + other->radius) {
        resolve_collision(particle, other);
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

  if (left_point < left_wall || right_point > right_wall) {
    particle->xvelocity *= -1;
  }
  if (top_point < top_wall || bottom_point > bottom_wall) {
    particle->yvelocity *= -1;
  }
}

void calculate_location(Circle *particle) {
  Uint32 time = SDL_GetTicks();
  float dt = (time - particle->lastupdated) / 1000.0f;
  particle->dy += particle->yvelocity * dt;
  particle->dx += particle->xvelocity * dt;

  // need to multiply with -1 to reverse the direction of movement
  particle->ycenter += (-1.0) * particle->yvelocity * dt;
  particle->xcenter += (-1.0) * particle->xvelocity * dt;

  particle->lastupdated = time;
}
