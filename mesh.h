#ifndef MESH_H
#define MESH_H
#include "vector.h"
#include "triangle.h"

// Define a constant for The number of vertices for our MESH
#define N_MESH_VERTICES 8

// in mesh.h we declare our array of vertices we use
// extern keyword becuase we will initialize and define
// them in mesh.c
extern vec3_t mesh_vertices[N_MESH_VERTICES];

// 6 cube faces, 2 triangles per face
#define N_MESH_FACES 12

//declare an array of faces
extern face_t mesh_faces[N_MESH_FACES];

// Pyramid, square base
#define P_MESH_VERTICES 5
extern vec3_t p_mesh_vertices[P_MESH_VERTICES];

#define P_MESH_FACES 5
extern face_t p_mesh_faces[P_MESH_FACES];

#endif