#include <stdint.h>
#include <math.h>

#include "meshes.h"

#define M_PI		3.14159265358979323846
#define M_PI_2		1.57079632679489661923

const v3t2n3_t cube_vertices[CUBE_VERTICES_NUM] =
{
    // front
    {{-1.f, 1.f, 1.f}, {0.f, 1.f}, { 0.f, 0.f, 1.f}},
    {{ 1.f, 1.f, 1.f}, {1.f, 1.f}, { 0.f, 0.f, 1.f}},
    {{ 1.f,-1.f, 1.f}, {1.f, 0.f}, { 0.f, 0.f, 1.f}},
    {{-1.f,-1.f, 1.f}, {0.f, 0.f}, { 0.f, 0.f, 1.f}},
    // back
    {{ 1.f, 1.f,-1.f}, {0.f, 1.f}, { 0.f, 0.f,-1.f}},
    {{-1.f, 1.f,-1.f}, {1.f, 1.f}, { 0.f, 0.f,-1.f}},
    {{-1.f,-1.f,-1.f}, {1.f, 0.f}, { 0.f, 0.f,-1.f}},
    {{ 1.f,-1.f,-1.f}, {0.f, 0.f}, { 0.f, 0.f,-1.f}},
    // top
    {{-1.f, 1.f,-1.f}, {0.f, 1.f}, { 0.f, 1.f, 0.f}},
    {{ 1.f, 1.f,-1.f}, {1.f, 1.f}, { 0.f, 1.f, 0.f}},
    {{ 1.f, 1.f, 1.f}, {1.f, 0.f}, { 0.f, 1.f, 0.f}},
    {{-1.f, 1.f, 1.f}, {0.f, 0.f}, { 0.f, 1.f, 0.f}},
    // bottom
    {{ 1.f,-1.f,-1.f}, {0.f, 1.f}, { 0.f,-1.f, 0.f}},
    {{-1.f,-1.f,-1.f}, {1.f, 1.f}, { 0.f,-1.f, 0.f}},
    {{-1.f,-1.f, 1.f}, {1.f, 0.f}, { 0.f,-1.f, 0.f}},
    {{ 1.f,-1.f, 1.f}, {0.f, 0.f}, { 0.f,-1.f, 0.f}},
    // left
    {{-1.f, 1.f,-1.f}, {0.f, 1.f}, {-1.f, 0.f, 0.f}},
    {{-1.f, 1.f, 1.f}, {1.f, 1.f}, {-1.f, 0.f, 0.f}},
    {{-1.f,-1.f, 1.f}, {1.f, 0.f}, {-1.f, 0.f, 0.f}},
    {{-1.f,-1.f,-1.f}, {0.f, 0.f}, {-1.f, 0.f, 0.f}},
    // right
    {{ 1.f, 1.f, 1.f}, {0.f, 1.f}, { 1.f, 0.f, 0.f}},
    {{ 1.f, 1.f,-1.f}, {1.f, 1.f}, { 1.f, 0.f, 0.f}},
    {{ 1.f,-1.f,-1.f}, {1.f, 0.f}, { 1.f, 0.f, 0.f}},
    {{ 1.f,-1.f, 1.f}, {0.f, 0.f}, { 1.f, 0.f, 0.f}}
};

const unsigned short cube_indices[CUBE_INDICES_NUM] =
{
    0, 3, 1,  1, 3, 2,  // front
    4, 7, 5,  5, 7, 6,  // back
    8,11, 9,  9,11,10,  // top
    12,15,13, 13,15,14, // bottom
    16,19,17, 17,19,18, // left
    20,23,21, 21,23,22  // right
};

const v3t2n3_t quad_vertices[4] =
{
    {{-1.f,  1.f, 0.f}, {0.f, 1.f}, {0.f, 0.f, 1.f}},
    {{ 1.f,  1.f, 0.f}, {1.f, 1.f}, {0.f, 0.f, 1.f}},
    {{ 1.f, -1.f, 0.f}, {1.f, 0.f}, {0.f, 0.f, 1.f}},
    {{-1.f, -1.f, 0.f}, {0.f, 0.f}, {0.f, 0.f, 1.f}},
};

const uint16_t quad_indices[6] =
{
    0, 3, 1,  1, 3, 2
};

struct fullscreen_vertex
{
    float x, y, z;
    float u, v;
};

const v3t2_t fullscreen_quad_vertices[6] =
{
    {-1.f, -1.f, 0.f,   0.f, 0.f},
    { 1.f, -1.f, 0.f,   1.f, 0.f},
    {-1.f,  1.f, 0.f,   0.f, 1.f},
    { 1.f, -1.f, 0.f,   1.f, 0.f},
    { 1.f,  1.f, 0.f,   1.f, 1.f},
    {-1.f,  1.f, 0.f,   0.f, 1.f}
};

GLuint new_cube()
{
    GLuint va = 0;
    GLuint vb = 0;
    GLuint ib = 0;

    glGenVertexArrays(1, &va);
    glBindVertexArray(va);

    glGenBuffers(1, &vb);
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glBufferData(GL_ARRAY_BUFFER, CUBE_VERTICES_NUM * sizeof(v3t2n3_t), cube_vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ib);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, CUBE_INDICES_NUM * sizeof(uint16_t), cube_indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(v3t2n3_t), (const GLvoid*)offsetof(v3t2n3_t, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(v3t2n3_t), (const GLvoid*)offsetof(v3t2n3_t, texcoord));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(v3t2n3_t), (const GLvoid*)offsetof(v3t2n3_t, normal));

    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    return va;
}

GLuint new_sphere(int rings, int sectors)
{
    float radius = 1.f;

    const float R = 1. / (float)(rings - 1);
    const float S = 1. / (float)(sectors - 1);
    int r, s;

    v3t2n3_t *vertices = (v3t2n3_t*)malloc(rings * sectors * sizeof(v3t2n3_t));
    v3t2n3_t *v = vertices;

    for(r = 0; r < rings; r++)
    {
        for(s = 0; s < sectors; s++)
        {
            const float y = sin(-M_PI_2 + M_PI * r * R );
            const float x = cos(2 * M_PI * s * S) * sin(M_PI * r * R);
            const float z = sin(2 * M_PI * s * S) * sin(M_PI * r * R);

            (*v).texcoord[0] = s * S;
            (*v).texcoord[1] = r * R;

            (*v).position[0] = x * radius;
            (*v).position[1] = y * radius;
            (*v).position[2] = z * radius;

            (*v).normal[0] = x;
            (*v).normal[1] = y;
            (*v).normal[2] = z;

            v++;
        }
    }

    uint16_t *indices = (uint16_t*)malloc(rings * sectors * 6 * sizeof(uint16_t));
    uint16_t *i = indices;

    for(r = 0; r < rings; r++)
    {
        for(s = 0; s < sectors; s++)
        {
            *i++ = r * sectors + s; // 0
            *i++ = r * sectors + (s + 1); // 1
            *i++ = (r + 1) * sectors + s; // 3
            *i++ = r * sectors + (s + 1); // 1
            *i++ = (r + 1) * sectors + (s + 1); // 2
            *i++ = (r + 1) * sectors + s; // 3
        }
    }

    GLuint va = 0;
    GLuint vb = 0;
    GLuint ib = 0;

    glGenVertexArrays(1, &va);
    glBindVertexArray(va);

    glGenBuffers(1, &vb);
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glBufferData(GL_ARRAY_BUFFER, rings * sectors * sizeof(v3t2n3_t), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ib);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, rings * sectors * 6 * sizeof(uint16_t), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(v3t2n3_t), (const GLvoid*)offsetof(v3t2n3_t, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(v3t2n3_t), (const GLvoid*)offsetof(v3t2n3_t, texcoord));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(v3t2n3_t), (const GLvoid*)offsetof(v3t2n3_t, normal));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    free(vertices);
    free(indices);

    return va;
}

GLuint new_quad()
{
    GLuint va = 0;
    GLuint vb = 0;
    GLuint ib = 0;

    glGenVertexArrays(1, &va);
    glBindVertexArray(va);

    glGenBuffers(1, &vb);
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glBufferData(GL_ARRAY_BUFFER, QUAD_VERTICES_NUM * sizeof(v3t2n3_t), quad_vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ib);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, QUAD_INDICES_NUM * sizeof(uint16_t), quad_indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(v3t2n3_t), (const GLvoid*)offsetof(v3t2n3_t, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(v3t2n3_t), (const GLvoid*)offsetof(v3t2n3_t, texcoord));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(v3t2n3_t), (const GLvoid*)offsetof(v3t2n3_t, normal));

    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    return va;
}
