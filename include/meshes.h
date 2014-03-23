#pragma once

#include <gl3/gl3platform.h>

#include "utils.h"

#define CUBE_VERTICES_NUM       24
#define CUBE_INDICES_NUM        36
#define QUAD_VERTICES_NUM       4
#define QUAD_INDICES_NUM        6

typedef struct v3t2n3_t
{
    float3 position;
    float2 texcoord;
    float3 normal;
} v3t2n3_t;

typedef struct v3t2_t
{
    float x, y, z;
    float u, v;
} v3t2_t;

extern const v3t2_t fullscreen_quad_vertices[6];

GLuint new_cube();
GLuint new_sphere(int rings, int sectors);
GLuint new_quad();
