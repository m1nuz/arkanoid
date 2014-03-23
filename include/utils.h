#pragma once

#define countof(x) (sizeof(x) / sizeof((x)[0]))

#define EMPTY_MATRIX4  { 0.0, 0.0, 0.0, 0.0,\
                         0.0, 0.0, 0.0, 0.0,\
                         0.0, 0.0, 0.0, 0.0,\
                         0.0, 0.0, 0.0, 0.0 }

#define IDENTITY_MATRIX4 { 1.0, 0.0, 0.0, 0.0,\
                           0.0, 1.0, 0.0, 0.0,\
                           0.0, 0.0, 1.0, 0.0,\
                           0.0, 0.0, 0.0, 1.0 }

typedef float float2[2];
typedef float float3[3];

struct AABB
{
    float3 min;
    float3 max;
};

typedef enum
{
    X_AXIS,
    Y_AXIS,
    Z_AXIS
} AXIS;

/* Multiply 4x4 matrix m1 by 4x4 matrix matrix2 and store the result in matrix1 */
void multiply4x4(float *matrix1, float *matrix2);

/* Generate a perspective view matrix using a field of view angle fov,
 * window aspect ratio, near and far clipping planes */
void perspective(float *matrix, float fov, float aspect, float nearz, float farz);

/* Perform translation operations on a matrix */
void translate(float *matrix, float x, float y, float z);

/* Rotate a matrix by an angle on a X, Y, or Z axis specified by the AXIS enum */
void rotate(float *matrix, float angle, AXIS axis);

/* Scale a matrix by value x, y, z and store the result in matrix */
void scale(float *matrix, float x, float y, float z);

void transpose(float *a);

int overlaps(struct AABB *a, struct AABB *b);
