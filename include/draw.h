#pragma once

#include "meshes.h"

void draw_cube(GLuint shader, const float3 color, const float *transform);
void draw_sphere(GLuint shader, const float3 color, const float *transform);
void draw_quad(GLuint shader, const float3 color, const float *transform);
void draw_fullscreen_quad(GLuint shader, GLuint sampler);
