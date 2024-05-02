#ifndef VECTOR_H
#define VECTOR_H

typedef struct {
  float x;
  float y;
} vec2_t;


typedef struct {
  float x;
  float y;
  float z;
} vec3_t;

vec3_t vec3_rotate_x(vec3_t v, float angle);
vec3_t vec3_rotate_y(vec3_t v, float angle);
vec3_t vec3_rotate_z(vec3_t v, float angle);

typedef struct {
  vec3_t position;
  vec3_t rotation;
  float fov_angle; //field of view
} camera_t;

float vec2_length (vec2_t v);
vec2_t vec2_add (vec2_t a, vec2_t b);
vec2_t vec2_subtract (vec2_t a, vec2_t b);  
vec2_t vec2_mul (vec2_t v, float scalar);
vec2_t vec2_div (vec2_t v, float scalar);
float vec2_dot (vec2_t, vec2_t);

////////////////////////////////////////////////////////////////////////////////////////
//Declarations of Vector 3d operation functions
////////////////////////////////////////////////////////////////////////////////////////
float vec3_length (vec3_t v);
vec3_t vec3_add (vec3_t a, vec3_t b);
vec3_t vec3_subtract (vec3_t a, vec3_t b);  
vec3_t vec3_mul (vec3_t v, float scalar);
vec3_t vec3_div (vec3_t v, float scalar);
vec3_t vec3_cross (vec3_t, vec3_t);
float vec3_dot (vec3_t, vec3_t);

// typedef struct {
//   int a, b, c;
// } face_t;

// typedef struct {
//   vec2_t points[3];
// } triangle_t;

typedef struct {
  float x, y, z, w ;
} vec4_t;


/* 4d functions  */
vec4_t vec4_from_vec3(vec3_t v);
vec3_t vec3_from_vec4(vec4_t v);

#endif