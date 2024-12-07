#include "vector.h"
#include <math.h>

float eucledean_dist(float x, float y) { return sqrtf(x * x + y * y); }

float mag_vector(Vector v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }

float rad_to_deg(float rad) { return rad * 180 / M_PI; }

float get_object_direction(Vector v) { return M_2_PI - atanf(v.y / v.x); }

Vector unit_vector(Vector v) {
  float mag = mag_vector(v);
  return (Vector){.x = v.x / mag, .y = v.y / mag, .z = v.z / mag};
}
Vector create_vector(float x, float y, float z) {
  return (Vector){.x = x, .y = y, .z = z};
}

float dot_product(Vector a, Vector b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector scale_vector(float s, Vector v) {
  return (Vector){.x = s * v.x, .y = s * v.y, .z = s * v.z};
}

Vector add_vectors(Vector a, Vector b) {
  return (Vector){.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z};
}
