#include "mesh.h"
#include "vector.h"

// New cube model based on cube vertices, rather than individual points
vec3_t mesh_vertices[N_MESH_VERTICES] = {
    {.x = -1, .y = -1, .z = -1}, //1
    {.x = -1, .y =  1, .z = -1}, //2
    {.x =  1, .y =  1, .z = -1}, //3
    {.x =  1, .y = -1, .z = -1}, //4
    {.x =  1, .y =  1, .z =  1}, //5
    {.x =  1, .y = -1, .z =  1}, //6
    {.x = -1, .y =  1, .z =  1}, //7
    {.x = -1, .y = -1, .z =  1}  //8
};

// Create every face of the cube with it's vector points
face_t mesh_faces[N_MESH_FACES] = {
    //front
    { .a = 1, .b = 2, .c = 3},
    { .a = 1, .b = 3, .c = 4},
    //right
    { .a = 4, .b = 3, .c = 5},
    { .a = 4, .b = 5, .c = 6},
    //back
    { .a = 6, .b = 5, .c = 7},
    { .a = 7, .b = 7, .c = 8},
    //left
    { .a = 8, .b = 7, .c = 2},
    { .a = 8, .b = 2, .c = 1},
    //top 
    { .a = 2, .b = 7, .c = 5},
    { .a = 2, .b = 5, .c = 3},
    //bottom
    { .a = 6, .b = 8, .c = 1},
    { .a = 6, .b = 1, .c = 4},
};

// vec3_t pyramid_mesh_vertices[P_MESH_VERTICES] = {
//     {.x = -1, .y = -1, .z = -1}, //1
//     {.x = -1, .y =  1, .z = -1}, //2
//     {.x =  -1, .y =  1, .z = 1}, //3
//     {.x =  1, .y = 1, .z = -1} //4
// };

// face_t pyramid_mesh_faces[P_MESH_FACES] = {
//     // face 1
//     { .a = 1, .b = 2, .c = 4},
//     // face 2
//     { .a = 4, .b = 2, .c = 3},
//     // face 3
//     { .a = 3, .b = 2, .c = 1},
//     // bottom (face 4)
//     { .a = 1, .b = 3, .c = 4}
// };

vec3_t p_mesh_vertices[P_MESH_VERTICES] = {
    { 0, 0, 0 },   // Bottom-left corner of the base
    { 1, 0, 0 },   // Bottom-right corner of the base
    { 1, 0, 1 },   // Top-right corner of the base
    { 0, 0, 1 },   // Top-left corner of the base
    { 0.5, 1, 0.5 } // Apex of the pyramid
};

face_t p_mesh_faces[P_MESH_FACES] = {
    { 1, 2, 5 },  // Front face
    { 2, 3, 5 },  // Right face
    { 3, 4, 5 },  // Back face
    { 4, 1, 5 },  // Left face
    { 1, 2, 3 },  // Base face (lower triangle)
    // { 3, 4, 1 }   // Base face (upper triangle)
};
