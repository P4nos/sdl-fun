#ifndef VECTOR_H
#define VECTOR_H

typedef struct Vector {
  float x;
  float y;
  float z;
} Vector;

Vector unit_vector(Vector v);
Vector create_vector(float x, float y, float z);
Vector scale_vector(float s, Vector v);
Vector add_vectors(Vector a, Vector b);
float dot_product(Vector a, Vector b);
float eucledean_dist(float x, float y);

#endif
