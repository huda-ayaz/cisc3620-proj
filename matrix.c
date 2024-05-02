#include "matrix.h"
#include "vector.h"
#include <math.h>

mat4_t mat4_identity(void) {
    mat4_t m = {{
        {1,0,0,0},
        {0,1,0,0},
        {0,0,1,0},
        {0,0,0,1}
    }};
    return m;
}

mat4_t mat4_make_scale(float sx, float sy, float sz) {
// | sx  0  0  0 |
// |  0 sy  0  0 |
// |  0  0 sz  0 |  
// |  0  0  0  1 |
    mat4_t m = mat4_identity();
    m.m[0][0] = sx;
    m.m[1][1] = sy;
    m.m[2][2] = sz;

    return m;
}

mat4_t mat4_make_translate(float tx, float ty, float tz) {
  // | 1  0  0  tx |
  // |  0 1  0  ty |
  // |  0  0 1  tz |  
  // |  0  0  0  1 |
    mat4_t M = mat4_identity();

    M.m[0][3] = tx;
    M.m[1][3] = ty;
    M.m[2][3] = tz;

    return M;
  }

mat4_t mat4_make_rotation_x(float angle) {
    float c = cos(angle);
    float s = sin(angle);

  // | 1  0   0  0 |
  // | 0  c  -s  0 |
  // | 0  s   c  0 |  
  // | 0  0   0  1 |

    mat4_t M = mat4_identity();
    M.m[1][1] = c;
    M.m[1][2] = -s;
    M.m[2][1] = s;
    M.m[2][2] = c;

    return M;
}


mat4_t mat4_make_rotation_y(float angle) {
    float c = cos(angle);
    float s = sin(angle);

  // |  c  0   s  0 |
  // |  0  1   0  0 |
  // | -s  s   c  0 |  
  // |  0  0   0  1 |

    mat4_t M = mat4_identity();
    M.m[0][0] = c;
    M.m[0][2] = -s;
    M.m[2][0] = s;
    M.m[2][2] = c;

    return M;
}

mat4_t mat4_make_rotation_z(float angle) {
  float c = cos(angle);
  float s = sin(angle); 

  // |  c  -s   0  0 |
  // |  s   c   0  0 |
  // |  0   0   0  0 |  
  // |  0   0   0  1 |

    mat4_t M = mat4_identity();
    M.m[0][0] = c;
    M.m[0][2] = -s;
    M.m[2][0] = s;
    M.m[2][2] = c;

    return M;
}

vec4_t mat4_mul_vec4(mat4_t M, vec4_t v)  {
    vec4_t result;
    result.x = M.m[0][0] * v.x + M.m[0][1] * v.y + M.m[0][2] * v.z + M.m[0][3] * v.w; 
    result.y = M.m[1][0] * v.x + M.m[1][1] * v.y + M.m[1][2] * v.z + M.m[1][3] * v.w;
    result.z = M.m[2][0] * v.x + M.m[2][1] * v.y + M.m[2][2] * v.z + M.m[2][3] * v.w;
    result.w  =M.m[3][0] * v.x + M.m[3][1] * v.y + M.m[3][2] * v.z + M.m[3][3] * v.w;

    return result;
}

// mat4_t mat4_mul_mat4(mat4_t A, mat4_t B) {
//   mat4_t M;
//   // apply dot product betwwen rows and colums for every position of the matrix.
//   // loop all rows and columns
//   for ( int i = 0; i < 4; i++ )
//     for ( int j = 0; j < 4; j++ )
//       M.m[i][j] = ( A.m[i][0] * B.m[0][j] ) +
//                   ( A.m[i][1] * B.m[1][j] ) +
//                   ( A.m[i][2] * B.m[2][j] ) +
//                   ( A.m[i][3] * B.m[3][j] );
//   return M;
// }